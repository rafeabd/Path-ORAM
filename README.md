# Path ORAM Implementation

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Installation](#installation)
- [Set-Up](#setup)
- [Components](#components)
- [Security](#security)
- [Performance](#performance)
- [rORAM Extension](#roram-extension)
- [Contributing](#contributing)

## Overview

Oblivious RAM (ORAM) protocols address the fundamental challenge of hiding access patterns to
data stored on untrusted servers. Even when data is encrypted, the sequence of memory locations
accessed can leak sensitive information about the underlying data and operations. This problem
is particularly relevant in outsourced storage scenarios where a client stores sensitive data on an
untrusted server.
The goal of ORAM is to ensure that the access pattern observed by the server is independent of
the actual pattern of data access by the client, thus preventing information leakage through access
pattern analysis. While traditional ORAM protocols focus on individual block accesses, many real-
world applications require efficient execution of range queries, where multiple adjacent blocks need
to be retrieved. Our work implements two ORAM protocols:

• Path ORAM: A foundational tree-based ORAM scheme that provides obliviousness for
individual block accesses

• Range ORAM (rORAM): An extension of Path ORAM that efficiently supports range
queries while maintaining obliviousness

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

## Set Up

### Basic Operations
The usage of Path ORAM and rORAM provides a very similar setup, with there only being slight differences between both forms.

## Path Oram - /Path-ORAM/path_oram_disc/main.cpp
The setup for the numbers of buckets and size of range query is on **main.cpp**

To set the total number of buckets, you simply change the following value.
line 23:
```cpp
//This would be interpreted at 2^22 buckets
    const int num_buckets_low = pow(2,22);
```

To set the source file you set the file path for your system. Set this according to your system, such as Linux, Mac, or Windows.
line 50:
```cpp
// Change this to the appropriate file path for your system
    string datasetPath = "/mnt/c/Users/Admis/Documents/Path_ORAM/tests/2^22.txt"; //Windows Subsystem for Linux
```

line 85:
This sets the range query sizes you would like to perform.
```cpp
// Define the range query sizes using exponents: 2^1, 2^4, 2^10
    vector<int> exponents = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
```

## rORAM - /Path-ORAM/rORAM/main.cpp
The setup for the numbers of buckets and size of range query is on **main.cpp**

To set the total number of buckets, you simply change the following value.
line 156:
```cpp
//This would be interpreted at 2^22 buckets
    const int num_buckets_low = pow(2,22);
```

line 160:
This sets the range query sizes you would like to perform.
```cpp
// Define the range query sizes using exponents: 2^1, 2^4, 2^10
    // Range query sizes to test: 2^1, 2^4, 2^10, 2^14
    const vector<int> range_sizes = {
        1 << 1,  // 2^1 = 2
        1 << 2,
        1 << 3,
        1 << 4,  // 2^4 = 16
```

Set the max range of your range
line 190:
```cpp
// Find max range needed (the largest of our test range sizes)
    int max_range_power = 4;
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
