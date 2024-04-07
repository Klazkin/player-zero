import os
import socket
import random
import numpy as np
import tensorflow as tf

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


class LossWriteCallback(tf.keras.callbacks.Callback):
    def __init__(self):
        super().__init__()

    def on_epoch_end(self, epoch, logs=None):
        with open("loss_history.txt", "a") as f:
            f.write(f"{logs.get('loss')}\n")


LOSS_CALLBACK = LossWriteCallback()


def blue(s):
    return "\033[94m{}\033[00m".format(s)


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

    print(blue(f"{board_stack.shape=}"))
    print(blue(f"{policy_mask_stack.shape=}"))
    print(blue(f"{policy_stack.shape=}"))
    print(blue(f"{value_stack.shape=}"))

    return board_stack, policy_mask_stack, policy_stack, value_stack


def load_train_save(generation: int):
    board_stack, policy_mask_stack, policy_stack, value_stack = load_data()
    print(blue('Loading model'))
    model = tf.keras.models.load_model(f"./player_zero_model_gen{generation - 1}/")

    model.compile(loss=['categorical_crossentropy', 'mean_squared_error'],
                  optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
                  metrics=['accuracy'])

    print(blue('Training'))
    _history = model.fit(
        x=[board_stack, policy_mask_stack],
        y=[policy_stack, value_stack],
        batch_size=BATCH_SIZE,
        epochs=TRAINING_EPOCHS,
        callbacks=LOSS_CALLBACK
    )
    print(blue('Saving model'))
    model_save_path = f"./player_zero_model_gen{generation}/"
    model.save(model_save_path)
    print(blue('Converting to onnx model'))
    # tf2onnx.convert(model_save_path, "../gaf6/player_zero.onnx")
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
