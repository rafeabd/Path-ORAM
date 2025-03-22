# Range ORAM Implementation

## Overview
This project implements the Range ORAM (Oblivious RAM) extension of Path ORAM system with client-server architecture, providing secure and efficient data access with hidden access patterns. The implementation expands on the previous support for encryption, dynamic data management, and secure client-server communication, and incorporates additional Range Oram (rORAM) features such as hierarchical storage support, access pattern tracking, and batch evictions.

## Previous Path ORAM Features
- Range ORAM (rORAM) Extension of Path ORAM 
- AES-256-CBC encryption
- Secure client-server architecture
- Random access pattern obfuscation
- Range query support
  
## Range ORAM Features
- Hierarchical storage support
- Access pattern tracking
- Background operations
- Write buffer management
- Batch evictions

## Project Structure
```
rORAM/
├── cpp/
│   ├── block.cpp
│   ├── bucket.cpp
│   ├── client.cpp
│   ├── encryption.cpp
│   ├── helper.cpp
│   ├── main.cpp
│   ├── oram.cpp
│   └── server.cpp
├── include/
│   ├── block.h
│   ├── bucket.h
│   ├── client.h
│   ├── config.h
│   ├── encryption.h
│   ├── helper.h
│   ├── oram.h
│   └── server.h
├── Makefile
├── readme.md
└── trees/
    
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
## rORAM - /Path-ORAM/rORAM/main.cpp
The setup for the numbers of buckets and size of range query is on **main.cpp**

First you need to ensure you have a folder called **trees** in rORAM_paper, as shown on the project structure. This stores your rORAM trees and it is not possible to build without this, if you don't have this, you need to create one.

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

Set the max range of your range query
line 190:
```cpp
// Find max range needed (the largest of our test range sizes)
    int max_range_power = 4;
```
## Building

To build your rORAM trees, you simply need to do following sequence of commands:
```cpp
    cd rORAM_paper
```
```cpp
    make
```
```cpp
    ./executable/testing
```
Your rORAM trees will be built in the trees folder, and the result should be shown on terminal

Do 
```cpp
    make clean
```
To remove the created objects.
## Common issue

When first running path_oram_disc, assuming you make, you may encounter the following issue:
```cpp
    terminate called after throwing an instance of 'std::runtime_error'
    what():  Failed to reopen file
    Aborted (core dumped)
```
This is likely due to you missing a trees folder in rORAM_paper. Creating a folder should resolve this.