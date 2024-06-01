with open("../data_headers.txt") as f:
    headers = list(line.rstrip("\n") for line in f)

def main():
    data = input()
    print(">>>")
    split_data = data.split(",")
    for ll, dd in zip(headers, split_data):
        print(ll, dd)


if __name__ == '__main__':
    main()
