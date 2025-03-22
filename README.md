# Path ORAM & rORAM Implementation

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


## Set Up

### Basic Operations
The usage of Path ORAM and rORAM provides a very similar setup, with there only being slight differences between both forms. More in depth descriptions are in their respective folders.



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
