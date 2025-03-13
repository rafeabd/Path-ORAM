# Range ORAM Implementation

## Overview
This project implements the Range ORAM (Oblivious RAM) extension of Path ORAM system with client-server architecture, providing secure and efficient data access with hidden access patterns. The implementation includes support for encryption, dynamic data management, and secure client-server communication.

## Features
- Range ORAM Extension of Path ORAM 
- AES-256-CBC encryption
- Secure client-server architecture
- Random access pattern obfuscation
- Range query support
- 

## Project Structure
```
path-oram/
├── include/
│   ├── block.h
│   ├── bucket.h
│   ├── bst.h
│   ├── client.h
│   ├── config.h
│   ├── encryption.h
│   └── server.h
├── src/
│   ├── block.cpp
│   ├── bucket.cpp
│   ├── bst.cpp
│   ├── client.cpp
│   ├── encryption.cpp
│   ├── main.cpp
│   └── server.cpp
└── tests/
    └── data/
```

## Requirements
- C++17 or higher
- OpenSSL library
- CMake 3.10 or higher

## Building
```bash
mkdir build
cd build
cmake ..
make
```

## Usage
```cpp
// Initialize the system
vector<unsigned char> key = generateEncryptionKey(32);
BucketHeap oram_tree(num_buckets, bucket_capacity, key);
Server server(num_blocks, bucket_capacity, oram_tree);
Client client(num_blocks, &server, key);

// Write data
client.access(1, block_id, "data");

// Read data
block result = client.access(0, block_id, "");

// Perform range query
vector<block> results = client.range_query(start_id, end_id);
```

## Key Components

### Block
Represents the fundamental data unit in the ORAM system. Each block contains:
- Unique identifier
- Target leaf position
- Data content
- Dummy flag

### Bucket
Container for blocks in the ORAM tree:
- Fixed capacity
- Block management
- Dummy block handling

### BucketHeap
Implements the binary tree structure:
- Path management
- Bucket organization
- Tree operations

### Client
Handles client-side operations:
- Position map management
- Stash handling
- Access operations
- Path retrieval/writing

### Server
Manages server-side operations:
- ORAM tree maintenance
- Path retrieval
- Bucket updates

## Security Features
- AES-256-CBC encryption
- Secure random number generation
- Access pattern hiding
- Dummy block padding

## Performance
- Access Time: O(log N)
- Space Overhead: O(N)
- Stash Size: O(log N)

## Extending to rORAM
The codebase includes preparation for rORAM extension:
- Hierarchical storage support
- Access pattern tracking
- Background operations
- Write buffer management

## Contributing
1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License
[MIT License](LICENSE)

## Contact
For questions and support, please open an issue in the repository.
