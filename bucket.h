/*
#ifndef BUCKET_H
#define BUCKET_H

#include "block.h"
#include <vector>

class Bucket {
private:
    std::vector<Block> blocks;
    const int Z;  // Fixed bucket size

public:
    explicit Bucket(int capacity = 4);
    bool addBlock(const Block& block);
    std::vector<Block> removeAllBlocks();
    const std::vector<Block>& getBlocks() const;
    bool hasSpace() const { return blocks.size() < Z; }
    size_t size() const { return blocks.size(); }
    int capacity() const { return Z; }
};

#endif
*/