import os
import socket

import keras
import numpy as np
import tensorflow as tf

from player_zero_builder import build_model, build_training_scheduler, build_early_stopper, LEARNING_RATE
from player_zero_data import load_ramdisk_data, clear_ramdisk_data, blue

# TRAINING CONF
TRAINING_EPOCHS = 150
BATCH_SIZE = 256

# SERVER CONF
SERVER_ADDRESS = "127.0.0.1"
SERVER_PORT = 22442
SIGNAL_SELF_PLAY_START = b"SPS"
SIGNAL_SELF_PLAY_END = b"SPE"
SIGNAL_NN_TRAINING_START = b"NTS"
SIGNAL_NN_TRAINING_END = b"NTE"
SIGNAL_END_OF_STREAM = b"EOS"

student_network_scale = 2


class LogTrainingCallback(tf.keras.callbacks.Callback):
    def __init__(self, path: str):
        self.path = path
        super().__init__()

    def on_epoch_end(self, epoch, logs=None):
        with open(self.path, "a") as f:
            l = lambda s: logs.get(s)
            f.write(f"{l('loss')},{l('policy_loss')},{l('value_loss')},{l('policy_accuracy')},{l('value_accuracy')}\n")


def load_train_save(generation: int):
    global student_network_scale

    def train(m: keras.Model, callbacks):
        print(blue("Adapting Normalization Layer..."))
        norm_layer: tf.keras.layers.Normalization = m.get_layer("norm")
        norm_layer.adapt(board_stack)

        lr = LEARNING_RATE
        print(blue("Learning rate set to:"), lr)
        m.optimizer.learning_rate = lr

        m.fit(
            x=[board_stack, mask_stack],
            y=[policy_stack, value_stack],
            batch_size=BATCH_SIZE,
            epochs=TRAINING_EPOCHS,
            callbacks=callbacks
        )

    print(blue('Loading data'))
    board_stack, mask_stack, _action_stack, policy_stack, value_stack = load_ramdisk_data()
    print(f"{board_stack.shape=}")
    print(f"{mask_stack.shape=}")
    print(f"{policy_stack.shape=}")
    print(f"{value_stack.shape=}")

    print(blue('Loading coach model'))
    model = tf.keras.models.load_model(f"./player_zero_model_gen{generation - 1}/")

    print(blue('Training coach model'))
    coach_early_stop = build_early_stopper()
    train(model, callbacks=[
        LogTrainingCallback("training_history.csv"),
        build_training_scheduler(),
        coach_early_stop,
    ])

    print(blue('Saving coach model'))
    model_save_path = f"./player_zero_model_gen{generation}/"
    model.save(model_save_path)

    print(blue('Loading student model'))
    next_model = tf.keras.models.load_model(f"./player_zero_model_gen{generation - 1}_next/")

    print(blue('Training student model'))
    train(next_model, callbacks=[
        LogTrainingCallback("training_history_next.csv"),
        build_training_scheduler(),
        build_early_stopper()
    ])

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

        print(blue('Training new student model'))
        train(next_model, callbacks=[
            LogTrainingCallback("training_history_replacement.csv"),
            build_training_scheduler(),
            build_early_stopper()
        ])

        print(blue('Saving new student model'))
        next_model_save_path = f"./player_zero_model_gen{generation}_next/"
        next_model.save(next_model_save_path)

    print(blue('Converting coach model to ONNX format'))
    os.system(f"python -m tf2onnx.convert --saved-model {model_save_path} --output ../gaf6/player_zero.onnx")


def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((SERVER_ADDRESS, SERVER_PORT))
    server_socket.listen()

    generation = input(blue("Enter Last trained generation (0 - default):"))

    try:
        generation = int(generation)
    except ValueError as _:
        generation = 0
    finally:
        print("Generation set to:", generation)

    if generation != 0:
        global student_network_scale
        try:
            student_network_scale = int(input(blue("Enter student network initial scale: (2 - default)")))
        except ValueError as _:
            student_network_scale = 2
        finally:
            print("Student network scale set to:", student_network_scale)


    print(blue("Awaiting connection on {}:{}".format(SERVER_ADDRESS, SERVER_PORT)))
    client_socket, client_address = server_socket.accept()
    print(blue("Linked with"), client_address)

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

            generation += 1
            print(blue("Training generation:"), generation)
            print(blue("Coach NN scale:"), student_network_scale - 1)
            print(blue("Student NN scale:"), student_network_scale)

            load_train_save(generation)
            clear_ramdisk_data()
            print(blue("NN training finished..."))
            client_socket.sendall(SIGNAL_NN_TRAINING_END)
            continue

        if data == SIGNAL_SELF_PLAY_START:
            print(blue("Self play started, waiting for it to finish..."))
            continue

        print(blue("Unrecognized data:"), data)

    print(blue("Closing connection with:"), client_address)
    client_socket.close()


def test_training():
    print(blue('Test training...'))
    print(blue('Loading data'))
    board_stack, mask_stack, action_stack, policy_stack, value_stack = load_ramdisk_data(use_random_samples=5)
    print(f"{board_stack.shape=}, {np.any(np.isnan(board_stack))=}")
    print(f"{mask_stack.shape=}, {np.any(np.isnan(mask_stack))=}")
    print(f"{action_stack.shape=}, {np.any(np.isnan(action_stack))=}")
    print(f"{policy_stack.shape=}, {np.any(np.isnan(policy_stack))=}")
    print(f"{value_stack.shape=}, {np.any(np.isnan(value_stack))=}")

    print(blue('Training model'))
    # model: keras.Model = build_multi_head(1, num_heads=1)  # tf.keras.models.load_model(f"./player_zero_model_gen1_next/")

    # model.optimizer.clipnorm = 1.0

    import datetime
    import time

    # model: keras.Model = build_model(1)
    model: keras.Model = tf.keras.models.load_model(f"./latest_test_model/")
    model.optimizer.learning_rate = 0.001
    # model.optimizer.learning_rate = 0.00005

    print(blue("Adapting data..."))
    norm_layer: tf.keras.layers.Normalization = model.get_layer("norm")
    norm_layer.adapt(board_stack)

    filename = f"tests/training_test_{datetime.datetime.now()}"

    with open(filename + ".txt", "w") as f:
        model.summary(print_fn=lambda x: f.write(x + '\n'))

    start_time = time.time()

    model.fit(
        x=[board_stack, mask_stack],
        y=[policy_stack, value_stack],
        batch_size=256,
        epochs=100,
        callbacks=[
            LogTrainingCallback(filename + ".csv"),
            build_training_scheduler()  # warmup_epochs=5, rate=-0.05
        ]
    )

    duration = time.time() - start_time

    with open(filename + ".txt", "a") as f:
        f.write(f"\n\nDuration: {duration}")

    model.save("latest_test_model")


if __name__ == '__main__':
    # test_training()
    main()
    # load_train_save(1)
# student_network_scale = 3
# load_train_save(3)

# import tensorflow as tf
# import numpy as np
#
# input_layer = tf.keras.layers.Input(shape=(10,))
# mask_layer = tf.keras.layers.Input(shape=(5,))
#
# x = tf.keras.layers.Dense(64, activation='relu', )(input_layer)
# x = tf.keras.layers.Dense(32, activation='relu')(x)
# x = tf.keras.layers.Dense(5)(x)
# x = tf.keras.layers.Softmax()(x)
#
# model = tf.keras.models.Model(inputs=[input_layer, mask_layer], outputs=x)
# model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])
#
# model.summary()
#
# X_train = [
#     np.random.rand(1000, 10),
#     np.array([np.array([1.0, 0.0, 1.0, 1.0, 0.0]) for _ in range(1000)])
# ]  # Example input data with 1000 samples and 10 features
#
# y_train = np.random.randint(0, 5, size=(1000,))
# y_train_one_hot = tf.keras.utils.to_categorical(y_train, num_classes=5)
# model.fit(X_train, y_train_one_hot, epochs=10, batch_size=32, validation_split=0.2)
