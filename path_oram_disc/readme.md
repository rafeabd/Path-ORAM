# Path ORAM Implementation

## Overview
This project implements a Path ORAM (Oblivious RAM) system with client-server architecture, providing secure and efficient data access with hidden access patterns. The implementation includes support for encryption, dynamic data management, and secure client-server communication.

## Features
- Complete Path ORAM implementation
- AES-256-CBC encryption
- Secure client-server architecture
- Random access pattern obfuscation
- Range query support
- Extensible for rORAM implementation

## Project Structure
```
path_oram_disc/
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
├── tests/
│   └── data/
└── tree/
```

## Requirements
- C++17 or higher
- OpenSSL library
- CMake 3.10 or higher

## Set-Up
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
    string datasetPath = "/mnt/c/Users/Admin/Documents/Path_ORAM/tests/2^22.txt"; //Windows Subsystem for Linux 
```

line 85:
This sets the range query sizes you would like to perform.
```cpp
// Define the range query sizes using exponents: 2^1, 2^4, 2^10
    vector<int> exponents = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
```

## Building

To build your Path ORAM tree, you simply need to do following sequence of commands:
```cpp
    cd path_oram_disc
```
```cpp
    make
```
```cpp
    ./executable/testing
```

Your Path ORAM tree will be built in the tree folder, and the result should be shown on terminal

## Common issue

When first running path_oram_disc, assuming you make, you may encounter the following issue:
```cpp
    terminate called after throwing an instance of 'std::runtime_error'
    what():  Failed to reopen file
    Aborted (core dumped)
```
This is likely due to one of two issues, either your file path is incorrect for your system or you are missing a tree folder in path_oram_disc. Resolving these issues should allow for it to build without issues.