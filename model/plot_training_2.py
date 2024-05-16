import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from training_server import TRAINING_EPOCHS

file = 'training_history_next.csv'

FONT_SIZE = 18


def plot_gen(ax, gen_num: int, tot: int):
    df = pd.read_csv(file, header=None)  # replace with .csv
    df = df[(gen_num - 1) * TRAINING_EPOCHS: gen_num * TRAINING_EPOCHS]
    x = np.arange(0, TRAINING_EPOCHS)

    y1 = df[0]
    y4 = df[3]

    ax.axhline(y=min(y1), color='#1F77B4', linestyle='-', linewidth=0.5)
    ax.axhline(y=max(y4), color='#52873B', linestyle='-', linewidth=0.5)

    txt_offset = 40

    ax.text(x=TRAINING_EPOCHS - txt_offset, y=min(y1) - 0.12, s=str(min(round(y1, 3))), color='#1F77B4',
            fontsize=FONT_SIZE)
    ax.text(x=TRAINING_EPOCHS - txt_offset, y=max(y4) + 0.02, s=str(max(round(y4, 3))), color='#52873B',
            fontsize=FONT_SIZE)

    names = ['loss', 'policy_loss', 'value_loss', 'policy_accuracy', 'value_accuracy', ]

    ax.plot(x, y1, '#1F77B4', label=names[0])
    ax.plot(x, y4, '#52873B', label=names[3])

    ax.set_xticks(np.arange(0, TRAINING_EPOCHS + 1, 25))
    ax.set_yticks(np.arange(0.0, 10.0, 0.5))
    ax.tick_params(axis='both', which='major', labelsize=FONT_SIZE)

    ax.set_ylim(0, 2.5)
    ax.set_title(f"Generation {gen_num}", fontsize=FONT_SIZE)

    if gen_num == 1:
        ax.legend(fontsize=FONT_SIZE)


def main():
    num_generations = 6
    fig, axes = plt.subplots(2, 3, figsize=(12, 10), sharex=True, sharey=True)

    for i, ax in enumerate(axes.flat):
        plot_gen(ax, i + 1, num_generations)

    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    main()
