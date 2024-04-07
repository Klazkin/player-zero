import os
import socket
import random

import keras.callbacks
import numpy as np
import tensorflow as tf

from player_zero_builder import build_model

# GAME CONF
BOARD_SIZE = 12
ACTIONS = 30

# DATA CONF
RANDOM_SAMPLES_PER_GAME = 5
TRAINING_EPOCHS = 100
BATCH_SIZE = 32
GENERATED_GAME_DATA_PATH = "/mnt/r/"

# SERVER CONF
SERVER_ADDRESS = "127.0.0.1"
SERVER_PORT = 22442
SIGNAL_SELF_PLAY_START = b"SPS"
SIGNAL_SELF_PLAY_END = b"SPE"
SIGNAL_NN_TRAINING_START = b"NTS"
SIGNAL_NN_TRAINING_END = b"NTE"
SIGNAL_END_OF_STREAM = b"EOS"

student_network_scale = 2


def blue(s):
    return "\033[94m{}\033[00m".format(s)


class CustomCallback(tf.keras.callbacks.Callback):
    def __init__(self, path: str):
        self.path = path
        super().__init__()

    def on_epoch_end(self, epoch, logs=None):
        with open(self.path, "a") as f:
            f.write(f"{logs.get('loss')},{logs.get('policy_loss')},{logs.get('value_loss')}\n")



def load_data():
    def load_sel_play_data(filepath: str):
        game_board_list = []
        game_policy_mask_list = []
        game_policy_list = []
        game_turn_factions = []

        winner: float
        with open(filepath, 'r') as f:
            while True:
                board = np.zeros(shape=(2592,), dtype=float)
                # BOARD_SIZE * BOARD_SIZE * (FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES))
                policy = np.zeros(shape=(4320,), dtype=float)
                policy_mask = np.zeros(shape=(4320,), dtype=float)
                # BOARD_SIZE * BOARD_SIZE * ACTION
                current_faction = None

                num_units = int(f.readline())
                for _ in range(num_units):
                    element_data = np.fromstring(f.readline(), dtype=float, sep=',')
                    element_index = round(element_data[0] * BOARD_SIZE + element_data[1]) * 18
                    board[element_index: element_index + len(element_data) - 2] = element_data[2:]

                    if element_data[5] == 1.0:
                        current_faction = 1.0 if element_data[3] == 1.0 else -1.0

                num_actions = int(f.readline())
                for _ in range(num_actions):
                    action_data = np.fromstring(f.readline(), dtype=float, sep=',')
                    action_index = round(action_data[0] * BOARD_SIZE + action_data[1]) * ACTIONS + round(action_data[2])
                    policy[action_index] = action_data[4]
                    policy_mask[action_index] = 1.0

                if num_actions == num_units == 0:
                    winner = float(f.readline())
                    break

                game_board_list.append(board)
                game_policy_mask_list.append(policy_mask)
                game_policy_list.append(policy)
                game_turn_factions.append(current_faction)

        game_iterator = zip(game_board_list, game_policy_mask_list, game_policy_list, game_turn_factions) \
            if len(game_turn_factions) <= RANDOM_SAMPLES_PER_GAME \
            else random.sample(list(zip(game_board_list, game_policy_mask_list, game_policy_list, game_turn_factions)),
                               k=RANDOM_SAMPLES_PER_GAME)

        for board, policy_mask, policy, turn_faction in game_iterator:
            if turn_faction is None:
                print(blue("No current unit on board?"))

            board_list.append(board)
            policy_mask_list.append(policy_mask)
            policy_list.append(policy)
            value_list.append(turn_faction * winner)

    board_list = []
    policy_mask_list = []
    policy_list = []
    value_list = []

    for file in os.listdir(GENERATED_GAME_DATA_PATH):
        if file.startswith("sim_"):
            load_sel_play_data(GENERATED_GAME_DATA_PATH + file)

    board_stack = np.vstack(board_list)
    policy_mask_stack = np.vstack(policy_mask_list)
    policy_stack = np.vstack(policy_list)
    value_stack = np.vstack(value_list)

    return board_stack, policy_mask_stack, policy_stack, value_stack


def load_train_save(generation: int):
    global student_network_scale

    print(blue('Loading data'))
    board_stack, policy_mask_stack, policy_stack, value_stack = load_data()

    print(blue('Loading coach model'))  # TODO use checkpoints instead
    model = tf.keras.models.load_model(f"./player_zero_model_gen{generation - 1}/")

    print(blue('Training coach model'))
    coach_early_stop = tf.keras.callbacks.EarlyStopping(
        monitor='loss',
        min_delta=0.005,
        patience=5,
        start_from_epoch=5
    )
    model.fit(
        x=[board_stack, policy_mask_stack],
        y=[policy_stack, value_stack],
        batch_size=BATCH_SIZE,
        epochs=TRAINING_EPOCHS,
        callbacks=[CustomCallback("training_history.csv"), coach_early_stop]
    )

    print(blue('Saving coach model'))
    model_save_path = f"./player_zero_model_gen{generation}/"
    model.save(model_save_path)

    print(blue('Loading student model'))  # TODO use checkpoints instead
    next_model = tf.keras.models.load_model(f"./player_zero_model_gen{generation - 1}_next/")

    print(blue('Training student model'))
    next_model.fit(
        x=[board_stack, policy_mask_stack],
        y=[policy_stack, value_stack],
        batch_size=BATCH_SIZE,
        epochs=TRAINING_EPOCHS,
        callbacks=[CustomCallback("training_history_next.csv")]
    )

    print(blue('Saving student model'))
    next_model_save_path = f"./player_zero_model_gen{generation}_next/"
    next_model.save(next_model_save_path)

    coach_converged = coach_early_stop.stopped_epoch != 0
    if coach_converged:
        student_network_scale += 1
        print(blue(f'Coach model converged! Scaling up student to {student_network_scale}'))
        print(blue('Saving student as the new coach model'))
        model = next_model
        model.save(model_save_path)
        print(blue('Building new student'))
        next_model = build_model(student_network_scale)
        print(blue('Fitting new student model'))
        next_model.fit(
            x=[board_stack, policy_mask_stack],
            y=[policy_stack, value_stack],
            batch_size=BATCH_SIZE,
            epochs=TRAINING_EPOCHS,
            callbacks=[CustomCallback("training_history_replacement.csv")]
        )
        print(blue('Saving new student model'))
        next_model_save_path = f"./player_zero_model_gen{generation}_next/"
        next_model.save(next_model_save_path)

    print(blue('Converting coach model to ONNX format'))
    os.system(f"python -m tf2onnx.convert --saved-model {model_save_path} --output ../gaf6/player_zero.onnx")


def remove_stale_data():
    for file in os.listdir(GENERATED_GAME_DATA_PATH):
        if file.startswith("sim_"):
            os.remove(GENERATED_GAME_DATA_PATH + file)


def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((SERVER_ADDRESS, SERVER_PORT))
    server_socket.listen()

    print(blue("Awaiting connection on {}:{}".format(SERVER_ADDRESS, SERVER_PORT)))
    client_socket, client_address = server_socket.accept()
    print(blue("Linked with"), client_address)

    generation = 0

    while True:
        data = client_socket.recv(3)

        if not data:
            print(blue('No data from client'))
            break

        print(blue("Received from client:"), data.decode())
        if data == SIGNAL_END_OF_STREAM:
            print(blue("Stopping server..."))
            break

        if data == SIGNAL_SELF_PLAY_END:
            print(blue("Self play ended, notify that training is starting..."))
            client_socket.sendall(SIGNAL_NN_TRAINING_START)
            print(blue("Training generation:"), generation)
            print(blue("Coach NN scale:"), student_network_scale - 1)
            print(blue("Student NN scale:"), student_network_scale)
            generation += 1
            load_train_save(generation)
            remove_stale_data()
            print(blue("NN training finished..."))
            client_socket.sendall(SIGNAL_NN_TRAINING_END)
            continue

        if data == SIGNAL_SELF_PLAY_START:
            print(blue("Self play started, waiting for it to finish..."))
            continue

        print(blue("Unrecognized data:"), data)

    print(blue("Closing connection with:"), client_address)
    client_socket.close()


if __name__ == '__main__':
    main()
