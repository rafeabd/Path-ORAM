/*
#ifndef SERVER_H
#define SERVER_H

#include "bucket.h"
#include <vector>

class Server {
private:
    std::vector<Bucket> tree;    // Binary tree stored as array
    const int L;                 // Tree height
    const int Z;                 // Bucket size

    std::vector<int> getPathToLeaf(int leaf) const;
    void initializeWithDummyBlocks();

public:
    Server(int num_blocks, int bucket_size = 4);
    std::vector<Block> readPath(int leaf);
    void writePath(int leaf, const std::vector<Block>& blocks);
};

#endif
*/