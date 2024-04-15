import os
import random
import numpy as np
from tqdm import tqdm

GENERATED_GAME_DATA_PATH = "/mnt/r/"

# GAME DATA CONF
BOARD_SIZE = 12
ACTIONS = 30
RANDOM_SAMPLES_PER_GAME = 10


def blue(s):
    return "\033[94m{}\033[00m".format(s)


# Factions
F_UNDEFINED, F_PLAYER, F_MONSTER = -1, 0, 1


def load_single_datapoint(board_list: list, mask_list: list, policy_list: list, value_list: list, filepath: str,
                          use_random_samples: bool = True):
    instance_board_list = []
    instance_mask_list = []
    instance_policy_list = []
    instance_turn_factions = []
    winning_faction: float

    with open(filepath, 'r') as f:
        while True:
            board = np.zeros(shape=(2592,), dtype=float)
            # BOARD_SIZE * BOARD_SIZE * (FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES))
            policy = np.zeros(shape=(4320,), dtype=float)
            mask = np.zeros(shape=(4320,), dtype=float)
            # BOARD_SIZE * BOARD_SIZE * ACTION
            current_faction = None

            num_units = int(f.readline())
            for _ in range(num_units):
                element_data = np.fromstring(f.readline(), dtype=int, sep=',')
                element_index = (element_data[0] * BOARD_SIZE + element_data[1]) * 18
                board[element_index: element_index + len(element_data) - 2] = element_data[2:]

                if element_data[5] == 1:  # IS_CONTROLLED flag
                    current_faction = F_PLAYER if element_data[3] == 1 else F_MONSTER  # CHECK F_MONSTER FLAG

            num_actions = int(f.readline())
            for _ in range(num_actions):
                action_data = np.fromstring(f.readline(), dtype=float, sep=',')
                action_index = round(action_data[0] * BOARD_SIZE + action_data[1]) * ACTIONS + round(action_data[2])
                policy[action_index] = action_data[4]
                mask[action_index] = 1.0

            if num_actions == num_units == 0:
                winning_faction = int(f.readline())
                if f.readline():
                    print(blue("ERROR: Not at end of file"))
                break
            else:
                instance_board_list.append(board)
                instance_mask_list.append(mask)
                instance_policy_list.append(policy)
                instance_turn_factions.append(current_faction)

    zipped_instance_lists = zip(instance_board_list, instance_mask_list, instance_policy_list, instance_turn_factions)
    instance_iterator = zipped_instance_lists \
        if len(instance_board_list) <= RANDOM_SAMPLES_PER_GAME or not use_random_samples \
        else random.sample(list(zipped_instance_lists), k=RANDOM_SAMPLES_PER_GAME)

    for board, mask, policy, turn_faction in instance_iterator:
        if turn_faction is None:
            print(blue("ERROR: No current unit on board?"))

        board_list.append(board)
        mask_list.append(mask)
        policy_list.append(policy)

        if turn_faction is None or turn_faction == F_UNDEFINED:
            print(blue("ERROR: Invalid Current Faction"), turn_faction)

        value: float
        if winning_faction == F_UNDEFINED:
            value = 0
        elif winning_faction == turn_faction:
            value = 1.0
        else:
            value = -1.0

        value_list.append(value)


def load_ramdisk_data():
    board_list = []
    mask_list = []
    policy_list = []
    value_list = []

    for file in tqdm(os.listdir(GENERATED_GAME_DATA_PATH)):
        if file.startswith("sim_"):
            load_single_datapoint(
                board_list, mask_list, policy_list, value_list,
                filepath=GENERATED_GAME_DATA_PATH + file,
            )

    board_stack = np.vstack(board_list)
    mask_stack = np.vstack(mask_list)
    policy_stack = np.vstack(policy_list)
    value_stack = np.vstack(value_list)

    return board_stack, mask_stack, policy_stack, value_stack


def clear_ramdisk_data():
    for file in os.listdir(GENERATED_GAME_DATA_PATH):
        if file.startswith("sim_"):
            os.remove(GENERATED_GAME_DATA_PATH + file)
