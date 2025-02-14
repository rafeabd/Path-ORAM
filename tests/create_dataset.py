def create_dataset(file_path, size):
    with open(file_path, 'wb') as f:
        for i in range(size):
            f.write(f"{i}, \n")

file_path = "/Users/rabdulali/Desktop/Path-ORAM/tests/2^20.txt"
size = 2  # 1 MB
create_dataset(file_path, size)