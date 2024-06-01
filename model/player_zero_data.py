import os
import random
import numpy as np

from tqdm import tqdm
from itertools import islice
from dataclasses import dataclass, field
from player_zero_builder import PZ_NUM_BOARD, PZ_NUM_POLICY

GENERATED_GAME_DATA_PATH = "/mnt/r/"

# GAME DATA CONF
BOARD_SIZE = 12
ACTIONS = 30
RANDOM_SAMPLES_PER_GAME = 7  # 5  # 7
LIMIT_GAMES_TO_LOAD = 15000

# Factions
F_UNDEFINED, F_PLAYER, F_MONSTER = -1, 0, 1

EXCLUDE_WITH_LOW_VISITS = False
ENABLE_H_V_FLIP = True
LOW_VISITS_THRESHOLD = 5000
TEMPERATURE = 1.0


def blue(s):
    return "\033[94m{}\033[00m".format(s)


@dataclass
class BundledData:
    board: list[np.ndarray] = field(default_factory=list)
    actions: list[np.ndarray] = field(default_factory=list)
    winning_faction: int = None


def load_single_datapoint(data_list: list[BundledData],
                          filepath: str,
                          use_random_samples: bool = True,
                          random_samples: int = RANDOM_SAMPLES_PER_GAME):
    winning_faction: float
    instance_bundles: list[BundledData] = []
    instance_winning_faction = None
    with open(filepath, 'r') as f:
        while True:
            bd = BundledData()

            num_units = int(f.readline())
            for _ in range(num_units):
                element_data = np.fromstring(f.readline(), dtype=int, sep=',')
                bd.board.append(element_data)

            num_actions = int(f.readline())
            for _ in range(num_actions):
                action_data = np.fromstring(f.readline(), dtype=float, sep=',')
                bd.actions.append(action_data)

            if num_actions == num_units == 0:
                instance_winning_faction = int(f.readline())
                break
            elif num_actions == 1:  # skip rows where there is only 1 option for an action
                continue
            else:
                instance_bundles.append(bd)

        if f.readline():
            print(blue("ERROR: Not at end of file"))
            return

    if instance_winning_faction not in (F_UNDEFINED, F_PLAYER, F_MONSTER):
        print(blue("Invalid winning faction id:"), instance_winning_faction)
        return

    for bd in instance_bundles:
        bd.winning_faction = instance_winning_faction

    if use_random_samples and len(instance_bundles) > random_samples:
        instance_bundles = random.sample(list(instance_bundles), k=random_samples)

    data_list.extend(instance_bundles)


def xyz_to_index(x: int, y: int, z: int, width: int, height: int, z_channels: int, flip_h: bool, flip_v: bool) -> int:
    if flip_h:
        x = (width - 1) - x
    if flip_v:
        y = (height - 1) - y
    return (x * height + y) * z_channels + z


def load_ramdisk_data(use_random_samples=True, use_random_flip=ENABLE_H_V_FLIP):
    data_list: list[BundledData] = []

    for file in tqdm(islice(os.listdir(GENERATED_GAME_DATA_PATH), LIMIT_GAMES_TO_LOAD)):
        if file.startswith("sim_"):
            load_single_datapoint(
                data_list,
                filepath=GENERATED_GAME_DATA_PATH + file,
                use_random_samples=use_random_samples
            )

    return convert_data_to_numpy(data_list, use_random_flip)


def convert_data_to_numpy(
        data_list: list[BundledData], use_random_flip=ENABLE_H_V_FLIP
) -> 'board_stack, mask_stack, action_list, policy_stack, value_stack':
    n = len(data_list)
    print(blue(f"Allocating data for {n} data bundles."))
    board_stack = np.zeros(shape=(n, PZ_NUM_BOARD), dtype=np.float32)
    mask_stack = np.zeros(shape=(n, PZ_NUM_POLICY), dtype=np.float32)
    action_list = np.zeros(shape=(1,))  # = np.zeros(shape=(n, ACTIONS * 2), dtype=float) # removed
    policy_stack = np.zeros(shape=(n, PZ_NUM_POLICY), dtype=np.float32)
    value_stack = np.zeros(shape=(n,), dtype=np.float32)

    for i, data in enumerate(data_list):
        current_faction = None
        flip_h = use_random_flip and random.random() > 0.5
        flip_v = use_random_flip and random.random() > 0.5

        # converting board data
        for element_data in data.board:
            element_index = xyz_to_index(
                x=element_data[0], y=element_data[1], z=0,
                width=BOARD_SIZE, height=BOARD_SIZE, z_channels=(18 + 30 * 2),
                flip_h=flip_h, flip_v=flip_v
            )
            board_stack[i, element_index: element_index + len(element_data) - 2] = element_data[2:]

            if element_data[5] == 1:  # IS_CONTROLLED flag
                current_faction = F_PLAYER if element_data[3] == 1 else F_MONSTER  # CHECK F_MONSTER FLAG

        # converting the actions
        actions = []
        for action_data in data.actions:
            action_index = xyz_to_index(
                x=int(action_data[0]), y=int(action_data[1]), z=int(action_data[2]),
                width=BOARD_SIZE, height=BOARD_SIZE, z_channels=ACTIONS,
                flip_h=flip_h, flip_v=flip_v
            )
            mask_stack[i, action_index] = 1.0
            action_visits = action_data[5]
            actions.append((action_index, action_visits))

        # based on code from https://github.com/suragnair/alpha-zero-general/blob/master/MCTS.py#L28
        actions = [(i, v ** (1.0 / TEMPERATURE)) for i, v in actions]
        weighted_visits_sum = float(sum(map(lambda a: a[1], actions)))

        for action_index, visits in actions:
            policy_stack[i, action_index] = visits / weighted_visits_sum

        # converting the value stack
        if data.winning_faction == F_UNDEFINED:
            value_stack[i] = 0.0
        elif data.winning_faction == current_faction:
            value_stack[i] = 1.0
        else:
            value_stack[i] = -1.0

    return board_stack, mask_stack, action_list, policy_stack, value_stack


def legacy_load_single_datapoint(
        board_list: list,
        mask_list: list,
        action_list: list,
        policy_list: list,
        value_list: list,
        filepath: str,
        use_random_samples: bool = True,
        use_random_flip: bool = ENABLE_H_V_FLIP,
        random_samples: int = RANDOM_SAMPLES_PER_GAME
):
    instance_board_list = []
    instance_mask_list = []
    instance_action_list = []
    instance_policy_list = []
    instance_turn_factions = []
    winning_faction: float

    with open(filepath, 'r') as f:
        while True:
            board = np.zeros(shape=(PZ_NUM_BOARD,), dtype=float)
            mask = np.zeros(shape=(PZ_NUM_POLICY,), dtype=float)
            actions = np.zeros(shape=(ACTIONS * 2), dtype=float)
            policy = np.zeros(shape=(PZ_NUM_POLICY,), dtype=float)

            current_faction = None
            total_num_visits = 0

            num_units = int(f.readline())
            for _ in range(num_units):
                element_data = np.fromstring(f.readline(), dtype=int, sep=',')
                element_index = (element_data[0] * BOARD_SIZE + element_data[1]) * (18 + 30 * 2)
                board[element_index: element_index + len(element_data) - 2] = element_data[2:]

                if element_data[5] == 1:  # IS_CONTROLLED flag
                    current_faction = F_PLAYER if element_data[3] == 1 else F_MONSTER  # CHECK F_MONSTER FLAG

                    for i in range(0, ACTIONS * 2):
                        actions[i] = element_data[i + 18]

            num_actions = int(f.readline())
            actions = []
            for _ in range(num_actions):
                action_data = np.fromstring(f.readline(), dtype=float, sep=',')
                action_index = round(action_data[0] * BOARD_SIZE + action_data[1]) * ACTIONS + round(action_data[2])
                # policy[action_index] = action_data[4] # old score based function
                mask[action_index] = 1.0
                action_visits = action_data[5]
                total_num_visits += action_visits
                actions.append((action_index, action_visits))

            # based on code from https://github.com/suragnair/alpha-zero-general/blob/master/MCTS.py#L28
            actions = [(i, v ** (1.0 / TEMPERATURE)) for i, v in actions]
            visits_sum = float(sum(map(lambda a: a[1], actions)))
            for index, visits in actions:
                policy[index] = visits / visits_sum

            if num_actions == num_units == 0:
                winning_faction = int(f.readline())
                if f.readline():
                    print(blue("ERROR: Not at end of file"))
                break
            elif num_actions == 1:
                continue  # skip rows where there is only 1 option for an action
            elif EXCLUDE_WITH_LOW_VISITS and total_num_visits < LOW_VISITS_THRESHOLD:
                continue
            else:
                instance_board_list.append(board)
                instance_mask_list.append(mask)
                instance_action_list.append(actions)
                instance_policy_list.append(policy)
                instance_turn_factions.append(current_faction)

    zipped_instance_lists = zip(
        instance_board_list, instance_mask_list, instance_action_list, instance_policy_list, instance_turn_factions)

    instance_iterator = zipped_instance_lists \
        if len(instance_board_list) <= random_samples or not use_random_samples \
        else random.sample(list(zipped_instance_lists), k=random_samples)

    for board, mask, action, policy, turn_faction in instance_iterator:
        if turn_faction is None or turn_faction == F_UNDEFINED:
            print(blue("ERROR: No current unit on board?"), turn_faction)

        if use_random_flip:
            board = board.reshape(BOARD_SIZE, BOARD_SIZE, 18 + ACTIONS * 2)
            mask = mask.reshape(BOARD_SIZE, BOARD_SIZE, ACTIONS)
            policy = policy.reshape(BOARD_SIZE, BOARD_SIZE, ACTIONS)

            if random.random() > 0.5:
                board = np.fliplr(board)
                mask = np.fliplr(mask)
                policy = np.fliplr(policy)

            if random.random() > 0.5:
                board = np.flipud(board)
                mask = np.flipud(mask)
                policy = np.flipud(policy)

            board = board.flatten()
            mask = mask.flatten()
            policy = policy.flatten()

        board_list.append(board)
        mask_list.append(mask)
        action_list.append(action)
        policy_list.append(policy)

        value: float
        if winning_faction == F_UNDEFINED:
            value = 0
        elif winning_faction == turn_faction:
            value = 1.0
        else:
            value = -1.0

        value_list.append(value)


def legacy_load_ramdisk_data(use_random_samples=True):
    board_list = []
    mask_list = []
    action_list = []
    policy_list = []
    value_list = []

    for file in tqdm(islice(os.listdir(GENERATED_GAME_DATA_PATH), LIMIT_GAMES_TO_LOAD)):
        if file.startswith("sim_"):
            legacy_load_single_datapoint(
                board_list, mask_list, action_list, policy_list, value_list,
                filepath=GENERATED_GAME_DATA_PATH + file,
                use_random_samples=use_random_samples
            )

    board_stack = np.vstack(board_list)
    mask_stack = np.vstack(mask_list)
    action_list = np.vstack(action_list)
    policy_stack = np.vstack(policy_list)
    value_stack = np.vstack(value_list)

    return board_stack, mask_stack, action_list, policy_stack, value_stack


def clear_ramdisk_data():
    for file in os.listdir(GENERATED_GAME_DATA_PATH):
        if file.startswith("sim_"):
            os.remove(GENERATED_GAME_DATA_PATH + file)
