import sys

def create_dataset(file_path, L):
    with open(file_path, 'w') as f:
        for i in range(L):
            f.write(f"{i},Data_for_block_{i}\n")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python generate_dataset.py <output_file> <L>")
        sys.exit(1)
    file_path = sys.argv[1]
    L = int(sys.argv[2])
    create_dataset(file_path, L)
