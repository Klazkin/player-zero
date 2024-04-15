import tensorflow as tf
from keras.layers import *
from keras.regularizers import *

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
VALUE_HEAD_UNITS = 64
DROPOUT_RATE = 0.3
LEARNING_RATE = 0.001
L1_REG = 0  # 1e-4
L2_REG = 0  # 1e-4


def res_block(x, scale: int):
    r = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False,
               kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    r = BatchNormalization(axis=1)(r)
    r = ReLU()(r)
    r = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False,
               kernel_regularizer=l1_l2(L1_REG, L2_REG))(r)
    r = BatchNormalization(axis=1)(r)
    r = ReLU()(r)  # redundant?
    r = Add()([r, x])
    r = ReLU()(r)
    return r


def gps_block(x, scale: int):
    # https://www.researchgate.net/figure/Proposed-GARB-The-structure-globally-aggregates-values-of-one-set-of-channels-to-bias_fig3_352720536
    x_filters = (CONV_FILTERS_PER_SCALE * scale)
    g_filters = x_filters // 3  # 1/3 of filters
    r_filters = g_filters * 2  # 2/3 of filters

    g = Conv2D(g_filters, kernel_size=1, strides=1, padding='same', use_bias=False,
               kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    g = BatchNormalization(axis=1)(g)
    g = ReLU()(g)
    g_max = GlobalMaxPooling2D()(g)
    g_mean = GlobalAvgPool2D()(g)
    g = Concatenate()([g_max, g_mean])
    g = Dense(units=r_filters, use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))(g)
    g = Reshape((1, 1, r_filters,))(g)
    g = UpSampling2D((BOARD_SIZE, BOARD_SIZE))(g)

    r = Conv2D(r_filters, kernel_size=1, strides=1, padding='same', use_bias=False,
               kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)

    r = Add()([r, g])
    r = BatchNormalization(axis=1)(r)
    r = ReLU()(r)
    r = Conv2D(x_filters, kernel_size=1, strides=1, padding='same', use_bias=False,
               kernel_regularizer=l1_l2(L1_REG, L2_REG))(r)
    r = Add()([r, x])
    return r


def build_model(scale: int):
    if scale < 1:
        raise RuntimeError("Scale must be above 0.")

    policy_shape = BOARD_SIZE * BOARD_SIZE * ACTIONS
    board_shape = BOARD_SIZE * BOARD_SIZE * (FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES)

    board_input = Input(name="board_input", shape=board_shape, dtype=float)
    mask_input = Input(name="mask_input", shape=policy_shape, dtype=float)

    x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES))(
        board_input)
    x = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False,
               kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    x = BatchNormalization(axis=1)(x)
    x = ReLU()(x)

    for _ in range(NUM_RES_BLOCKS_PER_SCALE * scale + 1):
        x = res_block(x, scale)

    # for i in range(NUM_RES_BLOCKS_PER_SCALE * scale + 1):
    #     if i % 2 == 0:
    #         x = res_block(x, scale)
    #     else:
    #         x = gps_block(x, scale)

    # value head
    vx = Conv2D(filters=2, kernel_size=1, padding='same', use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    vx = BatchNormalization(axis=1)(vx)
    vx = ReLU()(vx)
    vx = Flatten()(vx)
    vx = Dense(VALUE_HEAD_UNITS, use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))(vx)
    vx = BatchNormalization(axis=1)(vx)
    vx = ReLU()(vx)
    vx = Dropout(DROPOUT_RATE)(vx)
    vx = Dense(1, activation='tanh', name='value', kernel_regularizer=l1_l2(L1_REG, L2_REG))(vx)
    value_output_layer = vx

    # policy head
    # px = res_block(x, scale) modify below param
    px = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=1, padding='same', use_bias=False,
                kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    px = BatchNormalization(axis=1)(px)
    px = ReLU()(px)
    px = Conv2D(filters=ACTIONS, kernel_size=1, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG))(px)
    px = Flatten()(px)

    # px = Dense(units=policy_shape, kernel_regularizer=l1_l2(L1_REG, L2_REG))(px)

    def process_mask(mask_batch):
        # based on https://github.com/keras-team/keras/blob/f77b020e497a353b644df3aeebc97c831c8057fc/keras/layers/activations/softmax.py#L51
        # batch_size = tf.keras.backend.shape(mask_batch)[0]
        # const_ones = tf.keras.backend.constant(np.full((1, mask_shape), 1.0), dtype=float)
        # tiled_const_ones = tf.keras.backend.tile(const_ones, (batch_size, 1))
        # const_neg_large = tf.keras.backend.constant(np.full((1, mask_shape), -1e9), dtype=float)
        # tiled_const_neg_larges = tf.keras.backend.tile(const_neg_large, (batch_size, 1))
        # mask_batch = tiled_const_ones - mask_batch
        # mask_batch = tiled_const_neg_larges * mask_batch
        return -1e9 * (1.0 - mask_batch)

    mx = Lambda(process_mask, name='process_mask')(mask_input)
    px = Add()([px, mx])
    px = Softmax()(px)

    px = Layer(name='policy')(px)  # Dummy Buffer Layer, todo remove
    policy_output_layer = px

    model = tf.keras.models.Model(inputs=[board_input, mask_input],
                                  outputs=[policy_output_layer, value_output_layer])
    model.compile(loss=['categorical_crossentropy', 'mean_squared_error'],
                  optimizer=tf.keras.optimizers.Adam(learning_rate=LEARNING_RATE),
                  metrics=['accuracy'])
    return model


if __name__ == '__main__':
    import os

    model = build_model(scale=1)
    model.summary()

    tf.keras.utils.plot_model(
        model,
        show_shapes=True,
        show_dtype=True,
        show_layer_names=True,
        rankdir='TB',
        expand_nested=True,
        dpi=48,
        layer_range=None,
        show_layer_activations=True,
        show_trainable=True
    )
    model.save("./player_zero_model_gen0/")
    build_model(scale=2).save("./player_zero_model_gen0_next/")


    def remove_if_exists(path):
        if os.path.exists(path):
            os.remove(path)


    remove_if_exists("training_history.csv")
    remove_if_exists("training_history_next.csv")
    remove_if_exists("training_history_replacement.csv")

    os.system(f"python -m tf2onnx.convert --saved-model ./player_zero_model_gen0/ --output ../gaf6/player_zero.onnx")
