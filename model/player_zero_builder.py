import math
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
RES_BLOCK_KERNEL = 5
NUM_RES_BLOCKS_PER_SCALE = 2
VALUE_HEAD_UNITS = CONV_FILTERS_PER_SCALE
DROPOUT_RATE = 0.3

LEARNING_RATE_001 = 0.001
LEARNING_RATE_0005 = 0.0005
LEARNING_RATE_0002 = 0.0002
LEARNING_RATE_REDUCTION = -0.1

L1_REG = 0  # 1e-4
L2_REG = 1e-4

PZ_NUM_POLICY = BOARD_SIZE * BOARD_SIZE * ACTIONS
PZ_NUM_BOARD = BOARD_SIZE * BOARD_SIZE * (FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2)


class GlobalContextEncoder(Layer):
    def __init__(self, projection_dim):
        super().__init__()
        self.g_dense = Dense(
            units=projection_dim * 4,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )
        self.l_dense = Dense(
            units=projection_dim,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )
        self.activation = ReLU()
        self.batch_norm1 = BatchNormalization()
        self.batch_norm2 = BatchNormalization()
        self.g_pool = GlobalAveragePooling1D()
        self.g_reshape = Reshape((1, projection_dim * 4,))
        self.g_upsample = UpSampling1D((BOARD_SIZE * BOARD_SIZE))
        self.concat = Concatenate()

    def call(self, inputs, *args, **kwargs):
        g = self.g_dense(inputs)
        g = self.batch_norm1(g)
        g = self.activation(g)
        g = self.g_pool(g)
        g = self.g_reshape(g)
        g = self.g_upsample(g)

        x = self.concat([g, inputs])
        x = self.l_dense(x)
        x = self.batch_norm2(x)
        x = self.activation(x)
        return x


class ResBlock(Layer):
    def __init__(self, scale, filters=None, bias=False):
        filters = CONV_FILTERS_PER_SCALE * scale if filters is None else filters
        super().__init__()
        self.conv1 = Conv2D(filters=filters, kernel_size=RES_BLOCK_KERNEL, strides=1,
                            padding='same', use_bias=bias, kernel_regularizer=l1_l2(L1_REG, L2_REG))
        self.conv2 = Conv2D(filters=filters, kernel_size=RES_BLOCK_KERNEL, strides=1,
                            padding='same', use_bias=bias, kernel_regularizer=l1_l2(L1_REG, L2_REG))
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
        self.encoder = Dense(
            units=projection_dim,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )

    def call(self, inputs, *args, **kwargs):
        x = self.encoder(inputs)
        return x


def build_early_stopper() -> tf.keras.callbacks.EarlyStopping:
    return tf.keras.callbacks.EarlyStopping(
        monitor='loss',
        patience=150,  # TODO REVERT
        start_from_epoch=5,
        min_delta=0.0002
    )


def build_training_scheduler(warmup_epochs: int = 10):
    def scheduler(epoch, lr):
        if epoch < warmup_epochs:
            return lr
        else:
            return lr * math.exp(LEARNING_RATE_REDUCTION)

    return tf.keras.callbacks.LearningRateScheduler(scheduler)


def get_learning_rate(gen: int) -> float:
    if gen <= 2:
        return LEARNING_RATE_001
    elif gen <= 5:
        return LEARNING_RATE_0005
    else:
        return LEARNING_RATE_0002


def build_model(scale: int):
    if scale < 1:
        raise RuntimeError("Scale must be above 0.")

    board_input = Input(name="board_input", shape=PZ_NUM_BOARD, dtype=float)
    mask_input = Input(name="mask_input", shape=PZ_NUM_POLICY, dtype=float)

    x = Reshape(target_shape=(BOARD_SIZE * BOARD_SIZE,
                              FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2))(board_input)
    x = Normalization(name="norm")(x)
    x = BoardEncoder(projection_dim=CONV_FILTERS_PER_SCALE * scale)(x)
    x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, CONV_FILTERS_PER_SCALE * scale))(x)

    for _ in range(NUM_RES_BLOCKS_PER_SCALE * scale + 1):
        x = ResBlock(scale, bias=True)(x)

    # value head
    vx = Conv2D(filters=2, kernel_size=1, padding='same', use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Flatten()(vx)
    vx = Dense(VALUE_HEAD_UNITS, use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(vx)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Dropout(DROPOUT_RATE)(vx)
    vx = Dense(units=1, activation='tanh', use_bias=True, name='value', kernel_regularizer=l1_l2(L1_REG, L2_REG),
               kernel_initializer='zeros')(vx)
    value_output_layer = vx

    # policy head
    px = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale, kernel_size=RES_BLOCK_KERNEL, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    px = BatchNormalization()(px)
    px = ReLU()(px)
    px = Conv2D(filters=ACTIONS, kernel_size=RES_BLOCK_KERNEL, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG), kernel_initializer='zeros')(px)
    px = BatchNormalization()(px)
    #
    # heads = []
    # for action in range(ACTIONS):
    #     hx = Conv2D(filters=1, kernel_size=1, padding='same', kernel_regularizer=l1_l2(L1_REG, L2_REG))(px)
    #     hx = BatchNormalization()(hx)
    #     hx = ReLU()(hx)
    #     hx = Flatten()(hx)
    #     hx = Dense(units=1, kernel_regularizer=l1_l2(L1_REG, L2_REG))(hx)
    #     heads.append(hx)
    #
    # hx = Concatenate()(heads)
    # hx = Dense(units=30, kernel_regularizer=l1_l2(L1_REG, L2_REG))(hx)
    # hx = Reshape(target_shape=(1, 1, 30))(hx)
    # px = Add()([px, hx])
    px = Flatten()(px)

    # px = Dense(units=px.shape[-1], use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(px) TODO REWERT

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
                  optimizer=tf.keras.optimizers.Adam(learning_rate=get_learning_rate(0)),
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
    remove_if_exists("../gaf6/player_zero.onnx")

    os.system(f"python -m tf2onnx.convert --saved-model ./player_zero_model_gen0/ --output ../gaf6/player_zero.onnx")
