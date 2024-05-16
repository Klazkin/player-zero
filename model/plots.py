import matplotlib.pyplot as plt
import numpy as np

# Sample data
categories = ['Number of simulations', 'Rollout  lenght', 'Inference speed over 1000 games (s)', 'Victories']
values1 = [5000, 25, 3003299 / 1000, 425]  # Values for the first comparison
values2 = [3500, 10, 1854973 / 1000, 461]   # Values for the second comparison

# Create subplots
fig, axs = plt.subplots(1, 4, figsize=(12, 6))

colors = ['#6BAED6', '#FD8D3C']
# Plotting each pair of bars side-by-side in a single subplot
for i in range(4):
    axs[i].bar(np.arange(2), [values1[i], values2[i]], width=0.4, color=colors)
    axs[i].set_xticks(np.arange(2))
    axs[i].set_xticklabels(['MCTS', 'MCTS+WP'])
    axs[i].set_title(categories[i])
    axs[i].set_ylabel('')

plt.tight_layout()
plt.show()
