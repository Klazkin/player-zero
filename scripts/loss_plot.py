import matplotlib.pyplot as plt
import numpy as np

EPOCH_INCREMENTS = 100

with open('../model/loss_history.txt', 'r') as f:
    y = list(map(float, f.readline().split("\n")[:-1]))

x = range(len(y))

plt.plot(x, y)

plt.fill_between(x, y, color='lightblue', alpha=0.3)

for i in range(EPOCH_INCREMENTS, len(y), EPOCH_INCREMENTS):
    plt.axvline(x=i, color='gray', linestyle='--')


plt.xticks(np.arange(min(x), max(x)+1, 10))
plt.yticks(np.arange(0.0, max(y)+1, 1.0))

plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.title('Training Epochs Loss over multiple generations')
plt.show()
