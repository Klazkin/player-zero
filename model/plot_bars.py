import matplotlib.pyplot as plt
import numpy as np

# Data for the bar plot
categories = ['Total Victories', 'Total Time Taken (s)']
mcts1 = [139 + 150 + 131, (15828 + 19701 + 20045) / 1000]
mcts4 = [159 + 157 + 138, (56806 + 67277 + 69141) / 1000]
pz1 = [157 + 150 + 140, (24130 + 24165 + 31147) / 1000]
pz4 = [164 + 169 + 164, (28426 + 22968 + 22352) / 1000]

# Set the positions and width for the bars
x = np.arange(len(categories))  # the label locations
width = 0.1  # the width of the bars

# Create the plot
fig, ax = plt.subplots(figsize=(6, 6))

# Plot the bars
rects1 = ax.bar(x - 1.5*width, mcts1, width, label='MCTS 1000', color='#DF3E23')
rects2 = ax.bar(x - 0.5*width, mcts4, width, label='MCTS 4000', color='#59C135')
rects3 = ax.bar(x + 0.5*width, pz1, width, label='PlayerZero 1', color='#249FDE')
rects4 = ax.bar(x + 1.5*width, pz4, width, label='PlayerZero 4', color='#FFD541')

# Add labels, title, and custom x-axis tick labels
# ax.set_xlabel('Category')
# ax.set_ylabel('Values')
# ax.set_title('')
ax.set_xticks(x)
ax.set_xticklabels(categories, fontsize=12)
ax.tick_params(axis='y', labelsize=12)
ax.legend(fontsize=14)

# Adjust layout
fig.tight_layout()

# Display the plot
plt.show()
