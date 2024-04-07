import tensorflow as tf
from keras.layers import *

# RULE SPEC
BOARD_SIZE = 12
ACTIONS = 30
FACTIONS = 3  # FACTION_1, FACTION_2, NEUTRAL (ELEMENT)
IS_CONTROLLED = 1
ATTRIBUTES = 5
STATUES = 9  # should be an encoded process set?

# MODEL SPEC
CONV_FILTERS_PER_SCALE = 16
KERNEL = 3
NUM_RES_BLOCKS_PER_SCALE = 2
DROPOUT_RATE = 0.3
LEARNING_RATE = 0.001


def res_block(y, scale: int):
    r = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False)(y)
    r = BatchNormalization()(r)
    r = ReLU()(r)
    r = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False)(r)
    r = BatchNormalization()(r)
    r = ReLU()(r)
    r = Add()([r, y])
    r = ReLU()(r)
    return r


def build_model(scale: int):
    if scale < 1:
        raise RuntimeError("Scale must be above 0.")

    board_input_layer = Input(name="board_input",
                              shape=(BOARD_SIZE * BOARD_SIZE * (FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES)))
    policy_mask_input = Input(name="policy_mask_input", shape=(BOARD_SIZE * BOARD_SIZE * ACTIONS))

    x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES))(
        board_input_layer)
    x = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False)(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    for _ in range(NUM_RES_BLOCKS_PER_SCALE * scale):
        x = res_block(x, scale)

    # value head
    vx = Conv2D(filters=1, kernel_size=1, padding='same', use_bias=False)(x)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Flatten()(vx)
    vx = Dense(BOARD_SIZE * BOARD_SIZE, use_bias=False)(vx)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Dropout(DROPOUT_RATE)(vx)
    vx = Dense(1, activation='tanh', name='value')(vx)
    value_output_layer = vx

    # policy head
    px = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=1, padding='same', use_bias=False)(x)
    px = BatchNormalization()(px)
    px = ReLU()(px)
    px = Conv2D(filters=ACTIONS, kernel_size=1, padding='same', use_bias=True)(px)
    px = Flatten()(px)
    px = Multiply()([px, policy_mask_input])
    px = Softmax(name='policy')(px)
    policy_output_layer = px

    model = tf.keras.models.Model(inputs=[board_input_layer, policy_mask_input],
                                  outputs=[policy_output_layer, value_output_layer])

    model.compile(loss=['categorical_crossentropy', 'mean_squared_error'],
                  optimizer=tf.keras.optimizers.Adam(learning_rate=LEARNING_RATE),
                  metrics=['accuracy'])

    return model
