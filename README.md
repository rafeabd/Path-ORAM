# Path ORAM Implementation

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Components](#components)
- [Security](#security)
- [Performance](#performance)
- [rORAM Extension](#roram-extension)
- [Contributing](#contributing)

## Overview

This project implements a Path ORAM (Oblivious RAM) system with an extension to rORAM, providing secure data access with hidden access patterns. It features a complete client-server architecture with encryption support, and in addition to this, builds on this implementing rORAM.

## Features

* Full Path ORAM implementation
* AES-256-CBC encryption
* Client-server architecture
* Hidden access patterns
* Range query support
* rORAM extension support

## Project Structure

```
path-oram/
├── include/
│   ├── block.h          # Block structure definitions
│   ├── bucket.h         # Bucket management
│   ├── bst.h            # Binary tree implementation
│   ├── client.h         # Client operations
│   ├── config.h         # System configuration
│   ├── encryption.h     # Encryption utilities
│   └── server.h         # Server operations
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

* C++17 or higher
* OpenSSL library
* CMake 3.10 or higher

## Installation

1. Clone the repository:
```bash
git clone https://github.com/username/path-oram.git
cd path-oram
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Build the project:
```bash
cmake ..
make
```

## Usage

### Basic Operations

```cpp
// Initialize system components
vector<unsigned char> key = generateEncryptionKey(32);
BucketHeap oram_tree(num_buckets, bucket_capacity, key);
Server server(num_blocks, bucket_capacity, oram_tree);
Client client(num_blocks, &server, key);

// Write operation
client.access(1, block_id, "data");

// Read operation
block result = client.access(0, block_id, "");

// Range query
vector<block> results = client.range_query(start_id, end_id);
```

### Configuration

Modify `config.h` to adjust system parameters:
```cpp
const int bucket_size = 4;    // Blocks per bucket
const int block_size = 2;     // Block size
```

## Components

### Block
The fundamental data unit containing:
* Unique identifier
* Target leaf position
* Data content
* Dummy flag

### Bucket
Container for blocks with:
* Fixed capacity
* Block management
* Dummy block handling

### BucketHeap
Binary tree structure implementing:
* Path management
* Bucket organization
* Tree operations

### Client
Handles:
* Position map
* Stash management
* Access operations
* Path retrieval/writing

### Server
Manages:
* ORAM tree
* Path retrieval
* Bucket updates

## Security

### Features
* AES-256-CBC encryption
* Secure random number generation
* Hidden access patterns
* Dummy block padding

### Considerations
* Access pattern obfuscation
* Encryption key management
* Secure communication
* Timing attack prevention

## Performance

### Complexity
* Access Time: O(log N)
* Space Overhead: O(N)
* Stash Size: O(log N)

### Optimizations
* Bucket compression
* Path caching
* Parallel processing
* Background eviction

## rORAM Extension

### Planned Features
* Hierarchical storage
* Access pattern optimization
* Background operations
* Write buffer management

### Implementation Guide
1. Add hierarchical levels
2. Implement background merging
3. Add access tracking
4. Optimize block placement

## Contributing

1. Fork the repository
2. Create a feature branch
   ```bash
   git checkout -b feature/new-feature
   ```
3. Commit changes
   ```bash
   git commit -am 'Add new feature'
   ```
4. Push to branch
   ```bash
   git push origin feature/new-feature
   ```
5. Create Pull Request

### Acknowledgments

Path ORAM paper authors
Contributors and reviewers
OpenSSL team
