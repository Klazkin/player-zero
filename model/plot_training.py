import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from training_server import TRAINING_EPOCHS

df = pd.read_csv('training_history_next.csv', header=None)  # replace with .csv

x = df.index
y1 = df[0]
y2 = df[1]
y3 = df[2]
y4 = df[3]
y5 = df[4]

for i in range(TRAINING_EPOCHS, len(df), TRAINING_EPOCHS):
    plt.axvline(x=i, color='gray', linestyle='--', linewidth=0.5)

plt.axhline(y=0.5, color='gray', linestyle='-', linewidth=0.5)
plt.axhline(y=1.5, color='gray', linestyle='-', linewidth=0.5)

for generation in range(len(df) // TRAINING_EPOCHS + 1):
    gen_start = generation * TRAINING_EPOCHS
    gen_end = gen_start + 99

    names = ['loss', 'policy_loss', 'value_loss', 'policy_accuracy', 'value_accuracy', ]
    if generation != 0:
        names = ['_nolegend_'] * 5

    plt.plot(x[gen_start:gen_end], y1[gen_start:gen_end], '#1F77B4', label=names[0])
    plt.plot(x[gen_start:gen_end], y2[gen_start:gen_end], '#FF7F0E', label=names[1], linestyle='--')
    plt.plot(x[gen_start:gen_end], y3[gen_start:gen_end], '#2CA02C', label=names[2], linestyle='--')
    plt.plot(x[gen_start:gen_end], y4[gen_start:gen_end], '#3B5287', label=names[3])
    plt.plot(x[gen_start:gen_end], y5[gen_start:gen_end], '#B75C4E', label=names[4])

plt.xticks(np.arange(min(x), max(x) + 1, TRAINING_EPOCHS))
plt.yticks(np.arange(0.0, 10.0, 0.5))

plt.ylim(0, 2.5)
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.legend()
plt.title('Training Epochs Loss Over Multiple Generations')
plt.show()
