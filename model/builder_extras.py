from player_zero_builder import *
from keras_core import ops


class ResBoardEncoder(Layer):
    def __init__(self, projection_dim):
        super().__init__()
        self.dense1 = Dense(
            units=projection_dim,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )
        self.dense2 = Dense(
            units=projection_dim,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )

        self.activation = ReLU()
        self.add = Add()
        self.batch_norm1 = BatchNormalization()
        self.batch_norm2 = BatchNormalization()

    def call(self, inputs, *args, **kwargs):
        x1 = self.dense1(inputs)
        x1 = self.batch_norm1(x1)  # Todo use other form of normalization?
        x1 = self.activation(x1)
        x2 = self.dense2(x1)
        x2 = self.batch_norm2(x2)
        x = self.add([x1, x2])
        x = self.activation(x)
        return x


class GlobalContextEncoder(Layer):
    def __init__(self, projection_dim):
        super().__init__()
        self.g_dense = Dense(
            units=projection_dim * 4,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )
        self.l_dense = Dense(
            units=projection_dim * 2,
            use_bias=True,
            kernel_regularizer=l1_l2(L1_REG, L2_REG)
        )
        self.g_pool = GlobalAveragePooling2D()
        self.g_reshape = Reshape((1, 1, projection_dim * 4,))
        self.g_upsample = UpSampling2D((BOARD_SIZE, BOARD_SIZE))
        self.concat = Concatenate()

    def call(self, inputs, *args, **kwargs):
        g = self.g_dense(inputs)
        g = self.g_pool(g)
        g = self.g_reshape(g)
        g = self.g_upsample(g)

        x = self.concat([g, inputs])
        x = self.l_dense(x)
        return x


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
        self.g_dense = Dense(units=r_filters, use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))
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
        g = self.g_dense(g)  # Todo optimize the functions used
        g = self.g_reshape(g)
        g = self.g_upsample(g)

        r = self.r_conv1(x)
        r = self.r_add([r, g])
        r = self.r_batch_norm(r)
        r = self.activation(r)
        r = self.r_conv2(r)
        r = self.r_add([r, inputs])
        return r


def build_multi_head(scale: int, num_heads: int = 4):
    board_input = Input(name="board_input", shape=PZ_NUM_BOARD, dtype=float)
    mask_input = Input(name="mask_input", shape=PZ_NUM_POLICY, dtype=float)

    board_norm = Normalization(name="norm")(board_input)

    x = Reshape(target_shape=(BOARD_SIZE * BOARD_SIZE,
                              FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2))(board_norm)
    x = BoardEncoder(projection_dim=CONV_FILTERS_PER_SCALE * scale * num_heads)(x)

    heads = []
    for hi in range(num_heads):
        slice_start = CONV_FILTERS_PER_SCALE * scale * hi
        slice_end = CONV_FILTERS_PER_SCALE * scale * (hi + 1)
        hx = x[:, :, slice_start:slice_end]
        hx = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, CONV_FILTERS_PER_SCALE * scale),
                     name=f"head{hi}_slice{slice_start}_{slice_end}")(hx)

        hx = ResBlock(scale, bias=True)(hx)
        # hx = GpsBlock(scale)(hx)
        hx = ResBlock(scale, bias=True)(hx)
        hx = ResBlock(scale, bias=True)(hx)
        hx = ResBlock(scale, bias=True)(hx)
        hx = ResBlock(scale, bias=True)(hx)

        heads.append(hx)

    x = Concatenate()(heads)
    # value head
    vx = Conv2D(filters=2, kernel_size=1, padding='same', use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Flatten()(vx)
    vx = Dense(BOARD_SIZE * BOARD_SIZE * 2, use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(vx)
    vx = BatchNormalization()(vx)
    vx = ReLU()(vx)
    vx = Dropout(DROPOUT_RATE)(vx)
    vx = Dense(1, activation='tanh', use_bias=True, name='value', kernel_regularizer=l1_l2(L1_REG, L2_REG),
               kernel_initializer="zeros")(vx)
    value_output_layer = vx

    # policy head
    px = Conv2D(filters=CONV_FILTERS_PER_SCALE * scale * num_heads, kernel_size=1, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG))(x)
    px = BatchNormalization()(px)
    px = ReLU()(px)
    px = Conv2D(filters=ACTIONS, kernel_size=1, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG), kernel_initializer="zeros")(px)
    px = BatchNormalization()(px)
    px = Flatten()(px)
    px = Dense(units=px.shape[-1], use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(px)

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


def build_model_alternative(scale: int):
    board_input = Input(name="board_input", shape=PZ_NUM_BOARD, dtype=float)
    action_input = Input(name="action_input", shape=(ACTIONS * 2), dtype=float)

    x = Reshape(target_shape=(BOARD_SIZE * BOARD_SIZE,
                              FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2))(board_input)
    x = BoardEncoder(projection_dim=CONV_FILTERS_PER_SCALE * scale)(x)
    x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, CONV_FILTERS_PER_SCALE * scale))(x)
    x = ResBlock(scale)(x)
    x = ResBlock(scale)(x)
    # x = ResBlock(scale)(x)

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

    ax = action_input
    ax = Reshape((1, 1, ax.shape[-1],))(ax)
    ax = UpSampling2D((BOARD_SIZE, BOARD_SIZE))(ax)

    px = Concatenate()([px, ax])
    px = Dense(units=ACTIONS, use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(px)
    # TODO investigate adding more batch norms and res connections
    px = ResBlock(None, filters=ACTIONS)(px)
    px = Conv2D(filters=ACTIONS, kernel_size=1, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG), kernel_initializer="zeros")(px)
    px = BatchNormalization()(px)
    px = Flatten()(px)
    px = Softmax(name='policy')(px)

    policy_output_layer = px
    model = tf.keras.models.Model(inputs=[board_input, action_input],
                                  outputs=[policy_output_layer, value_output_layer])
    model.compile(loss=['categorical_crossentropy', 'mean_squared_error'],
                  optimizer=tf.keras.optimizers.Adam(learning_rate=LEARNING_RATE),
                  metrics=['accuracy'])
    return model


def build_hybrid_model(scale: int):
    board_input = Input(name="board_input", shape=PZ_NUM_BOARD, dtype=float)
    mask_input = Input(name="mask_input", shape=PZ_NUM_POLICY, dtype=float)
    action_input = Input(name="action_input", shape=(ACTIONS * 2), dtype=float)

    board_input_norm = Normalization(name="norm")(board_input)

    x = Reshape(target_shape=(BOARD_SIZE * BOARD_SIZE,
                              FACTIONS + IS_CONTROLLED + ATTRIBUTES + STATUES + ACTIONS * 2))(board_input_norm)
    x = ResBoardEncoder(projection_dim=CONV_FILTERS_PER_SCALE * scale)(x)
    # x = BoardEncoder(projection_dim=CONV_FILTERS_PER_SCALE * scale)(x)
    x = Reshape(target_shape=(BOARD_SIZE, BOARD_SIZE, CONV_FILTERS_PER_SCALE * scale))(x)

    x = ResBlock(scale)(x)
    x = GpsBlock(scale)(x)
    x = ResBlock(scale)(x)

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

    ax = action_input
    ax = Reshape((1, 1, ax.shape[-1],))(ax)
    ax = UpSampling2D((BOARD_SIZE, BOARD_SIZE))(ax)

    px = Concatenate()([px, ax])
    px = ResBoardEncoder(projection_dim=ACTIONS)(px)
    # px = Dense(units=ACTIONS, use_bias=True, kernel_regularizer=l1_l2(L1_REG, L2_REG))(px)
    px = Conv2D(filters=ACTIONS, kernel_size=1, padding='same', use_bias=True,
                kernel_regularizer=l1_l2(L1_REG, L2_REG), kernel_initializer="zeros")(px)
    px = BatchNormalization()(px)
    px = Flatten()(px)

    def process_mask(mask_batch):
        # based on https://github.com/keras-team/keras/blob/f77b020e497a353b644df3aeebc97c831c8057fc/keras/layers/activations/softmax.py#L51
        return -1e9 * (1.0 - mask_batch)

    mx = Lambda(process_mask, name='process_mask')(mask_input)
    px = Add()([px, mx])
    px = Softmax(name='policy')(px)

    policy_output_layer = px
    model = tf.keras.models.Model(inputs=[board_input, mask_input, action_input],
                                  outputs=[policy_output_layer, value_output_layer])
    model.compile(loss=['categorical_crossentropy', 'mean_squared_error'],
                  optimizer=tf.keras.optimizers.Adam(learning_rate=LEARNING_RATE),
                  metrics=['accuracy'])
    return model


def focal_loss(y_true, y_pred):
    alpha = 0.25
    gamma = 2

    y_pred = ops.convert_to_tensor(y_pred)
    y_true = ops.cast(y_true, y_pred.dtype)

    # if label_smoothing:
    #     y_true = self._smooth_labels(y_true)

    # if from_logits:
    #     y_pred = ops.sigmoid(y_pred)

    cross_entropy = ops.binary_crossentropy(y_true, y_pred)

    alpha = ops.where(
        ops.equal(y_true, 1.0), alpha, (1.0 - alpha)
    )

    pt = y_true * y_pred + (1.0 - y_true) * (1.0 - y_pred)
    loss = (
            alpha
            * ops.cast(ops.power(1.0 - pt, gamma), alpha.dtype)
            * ops.cast(cross_entropy, alpha.dtype)
    )
    # In most losses you mean over the final axis to achieve a scalar
    # Focal loss however is a special case in that it is meant to focus on
    # a small number of hard examples in a batch. Most of the time this
    # comes in the form of thousands of background class boxes and a few
    # positive boxes.
    # If you mean over the final axis you will get a number close to 0,
    # which will encourage your model to exclusively predict background
    # class boxes.
    return ops.sum(loss, axis=-1)


def multi_category_focal_loss2(gamma=2., alpha=.25):
    """
    https://github.com/monkeyDemon/AI-Toolbox/blob/master/computer_vision/image_classification_keras/loss_function/focal_loss.py
    focal loss for multi category of multi label problem
    适用于多分类或多标签问题的focal loss

    alpha控制真值y_true为1/0时的权重
        1的权重为alpha, 0的权重为1-alpha
    当你的模型欠拟合，学习存在困难时，可以尝试适用本函数作为loss
    当模型过于激进(无论何时总是倾向于预测出1),尝试将alpha调小
    当模型过于惰性(无论何时总是倾向于预测出0,或是某一个固定的常数,说明没有学到有效特征)
        尝试将alpha调大,鼓励模型进行预测出1。

    Usage:
     model.compile(loss=[multi_category_focal_loss2(alpha=0.25, gamma=2)], metrics=["accuracy"], optimizer=adam)
    """
    epsilon = 1.e-7
    gamma = float(gamma)
    alpha = tf.constant(alpha, dtype=tf.float32)

    def multi_category_focal_loss2_fixed(y_true, y_pred):
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.clip_by_value(y_pred, epsilon, 1. - epsilon)

        alpha_t = y_true * alpha + (tf.ones_like(y_true) - y_true) * (1 - alpha)
        y_t = tf.multiply(y_true, y_pred) + tf.multiply(1 - y_true, 1 - y_pred)
        ce = -tf.math.log(y_t)
        weight = tf.pow(tf.subtract(1., y_t), gamma)
        fl = tf.multiply(tf.multiply(weight, ce), alpha_t)
        loss = tf.reduce_mean(fl)
        return loss

    return multi_category_focal_loss2_fixed


if __name__ == '__main__':
    model = build_multi_head(1)
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
