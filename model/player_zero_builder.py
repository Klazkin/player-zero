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
CONV_FILTERS_PER_SCALE = 32
RES_BLOCK_KERNEL = 5
NUM_RES_BLOCKS_PER_SCALE = 2
VALUE_HEAD_UNITS = 32
DROPOUT_RATE = 0.3
LEARNING_RATE = 0.001
L1_REG = 0  # 1e-4
L2_REG = 1e-4

PZ_NUM_POLICY = BOARD_SIZE * BOARD_SIZE * ACTIONS
PZ_NUM_BOARD = BOARD_SIZE * BOARD_SIZE * (FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2)


class ResBlock(Layer):
    def __init__(self, scale):
        super().__init__()
        self.conv1 = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=RES_BLOCK_KERNEL, strides=1,
                            padding='same', use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))
        self.conv2 = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=RES_BLOCK_KERNEL, strides=1,
                            padding='same', use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))
        self.batch_norm1 = BatchNormalization()
        self.batch_norm2 = BatchNormalization()
        self.activation = ReLU()
        self.add = Add()

    def call(self, inputs, *args, **kwargs):
        r = self.conv1(inputs)
        r = self.batch_norm1(r)
        r = self.activation(r)
        r = self.conv2(r)
        r = self.batch_norm2(r)
        r = self.activation(r)  # redundant?
        r = self.add([r, inputs])
        r = self.activation(r)
        return r


class BoardEncoder(Layer):
    def __init__(self, projection_dim):
        super().__init__()
        self.encoding = Dense(
            units=projection_dim,
            use_bias=True,
            name='board_encoding',
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )

    def call(self, inputs, *args, **kwargs):
        return self.encoding(inputs)


class GpsBlock(Layer):
    def __init__(self, scale):
        super().__init__()
        x_filters = (CONV_FILTERS_PER_SCALE * scale)
        g_filters = x_filters // 3  # 1/3 of filters
        r_filters = g_filters * 2  # 2/3 of filters

        self.x_batch_norm = BatchNormalization()

        self.g_conv = Conv2D(g_filters, kernel_size=1, strides=1, padding='same', use_bias=False,
                             kernel_regularizer=l1_l2(L1_REG, L2_REG))

        self.g_batch_norm = BatchNormalization()
        self.g_max = GlobalMaxPooling2D()
        self.g_mean = GlobalAvgPool2D()
        self.g_concat = Concatenate()
        self.g_dense = Dense(units=r_filters, use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))
        self.g_reshape = Reshape((1, 1, r_filters,))
        self.g_upsample = UpSampling2D((BOARD_SIZE, BOARD_SIZE))

        self.r_conv1 = Conv2D(r_filters, kernel_size=RES_BLOCK_KERNEL, strides=1, padding='same', use_bias=False,
                              kernel_regularizer=l1_l2(L1_REG, L2_REG))
        self.r_conv2 = Conv2D(x_filters, kernel_size=RES_BLOCK_KERNEL, strides=1, padding='same', use_bias=False,
                              kernel_regularizer=l1_l2(L1_REG, L2_REG))
        self.r_batch_norm = BatchNormalization()
        self.r_add = Add()
        self.activation = ReLU()

    def call(self, inputs, *args, **kwargs):
        x = self.x_batch_norm(inputs)
        x = self.activation(x)

        g = self.g_conv(x)
        g = self.g_batch_norm(g)
        g = self.activation(g)
        g = self.g_concat([self.g_max(g), self.g_mean(g)])
        g = self.g_dense(g)
        g = self.g_reshape(g)
        g = self.g_upsample(g)

        r = self.r_conv1(x)
        r = self.r_add([r, g])
        r = self.r_batch_norm(r)
        r = self.activation(r)
        r = self.r_conv2(r)
        r = self.r_add([r, inputs])
        return r


def build_model(scale: int):
    if scale < 1:
        raise RuntimeError("Scale must be above 0.")

    board_input = Input(name="board_input", shape=PZ_NUM_BOARD, dtype=float)
    mask_input = Input(name="mask_input", shape=PZ_NUM_POLICY, dtype=float)

    x = Reshape(target_shape=(BOARD_SIZE * BOARD_SIZE,
                              FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2))(board_input)
    x = BoardEncoder(projection_dim=CONV_FILTERS_PER_SCALE * scale)(x)
    x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, CONV_FILTERS_PER_SCALE * scale))(x)

    # Todo include batch norm after encoding?
    # x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES))(
    #     board_input)
    # x = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=KERNEL, strides=1, padding='same', use_bias=False,
    #            kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    # x = BatchNormalization()(x)
    # x = ReLU()(x)

    # for _ in range(NUM_RES_BLOCKS_PER_SCALE * scale + 1):
    #     x = ResBlock(scale)(x)

    for i in range(NUM_RES_BLOCKS_PER_SCALE * scale + 1):  # TODO temp removal
        if i % 2 == 0:
            x = ResBlock(scale)(x)
        else:
            x = GpsBlock(scale)(x)

    # value head
    vx = Conv2D(filters=2, kernel_size=1, padding='same', use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Flatten()(vx)
    vx = Dense(VALUE_HEAD_UNITS, use_bias=False, kernel_regularizer=l1_l2(L1_REG, L2_REG))(vx)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Dropout(DROPOUT_RATE)(vx)
    vx = Dense(1, activation='tanh', use_bias=True, name='value', kernel_regularizer=l1_l2(L1_REG, L2_REG),
               kernel_initializer="zeros")(vx)
    value_output_layer = vx

    # policy head
    px = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=1, padding='same', use_bias=False,
                kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    px = BatchNormalization()(px)
    px = ReLU()(px)
    px = Conv2D(filters=ACTIONS, kernel_size=1, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG), kernel_initializer="zeros")(px)
    px = Flatten()(px)

    def process_mask(mask_batch):
        # based on https://github.com/keras-team/keras/blob/f77b020e497a353b644df3aeebc97c831c8057fc/keras/layers/activations/softmax.py#L51
        return -1e9 * (1.0 - mask_batch)

    mx = Lambda(process_mask, name='process_mask')(mask_input)
    px = Add()([px, mx])
    px = Softmax(name='policy')(px)
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
