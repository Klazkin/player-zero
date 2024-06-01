from player_zero_data import GENERATED_GAME_DATA_PATH
import os
from tqdm import tqdm


def main():
    bad_files = []

    for file in tqdm(os.listdir(GENERATED_GAME_DATA_PATH)):
        if file.startswith("sim_"):
            with open(os.path.join(GENERATED_GAME_DATA_PATH, file), "r") as f:
                try:
                    content: str = f.read()
                    check = any(content.endswith(p) for p in ("0\n0\n-1\n", "0\n0\n1\n", "0\n0\n0\n"))
                    if check is False:
                        bad_files.append(file)

                except UnicodeDecodeError:
                    bad_files.append(file)

    if len(bad_files) == 0:
        print("All files valid.")

    for file in bad_files:
        print("removing...", file)
        os.remove(os.path.join(GENERATED_GAME_DATA_PATH, file))


if __name__ == '__main__':
    main()
