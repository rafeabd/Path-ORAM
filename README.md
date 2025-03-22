# Path ORAM & rORAM Implementation

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
├── altORAMs/
│   ├── path_oram
│   └── rORAM
│
├── path_oram_disc/
│
├── rORAM_paper/
│
├── tests/
└── README.md
```

## Requirements

* C++17 or higher
* OpenSSL library
* CMake 3.10 or higher

## Basic Operations & Components
The usage of Path ORAM and rORAM provides a very similar setup, with there only being slight differences between both forms. More in depth descriptions are in their respective folders.

For our *Path ORAM* implementation that was used for our testing, please navigate to **path_oram_disc** folder.

For our *rORAM* implementation that was used for our testing, please navigate to **rORAM_paper** folder.

Implementations which are more experimental, and not officially apart of our project, are in the **altORAMs** folder. This include a regular Path ORAM that doesn't operate on disc and a rORAM that uses a technique we created called "chunked eviction" is in this folder.

More in depth information regarding the theory behind our implementations is available to read on our final report regarding the development of this program, please refer to the **reports** folder.

## Basic Components
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
