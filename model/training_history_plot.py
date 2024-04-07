import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

EPOCH_INCREMENTS = 100

df = pd.read_csv('training_history.csv', header=None)  # replace with .csv

x = df.index
y1 = df[0]
y2 = df[1]
y3 = df[2]

for i in range(EPOCH_INCREMENTS, len(df), EPOCH_INCREMENTS):
    plt.axvline(x=i, color='gray', linestyle='--', linewidth=0.5)

plt.axhline(y=0.5, color='gray', linestyle='-', linewidth=0.5)


for generation in range(len(df) // EPOCH_INCREMENTS + 1):
    gen_start = generation * EPOCH_INCREMENTS
    gen_end = gen_start + 99
    plt.plot(x[gen_start:gen_end], y1[gen_start:gen_end], '#1F77B4', label='loss')
    plt.plot(x[gen_start:gen_end], y2[gen_start:gen_end], '#FF7F0E', label='policy_loss')
    plt.plot(x[gen_start:gen_end], y3[gen_start:gen_end], '#2CA02C', label='value_loss')

plt.xticks(np.arange(min(x), max(x) + 1, EPOCH_INCREMENTS))
plt.yticks(np.arange(0.0, 10.0, 0.5))

plt.ylim(0, 4.5)
plt.xlabel('Epoch')
plt.ylabel('Loss')
# plt.legend()
plt.title('Training Epochs Loss Over Multiple Generations')
plt.show()
