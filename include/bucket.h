#ifndef BUCKET_H
#define BUCKET_H

#include "block.h"
#include <vector>

class Bucket {
private:
    std::vector<block> blocks;
    const int Z;  // Fixed bucket size

public:
    explicit Bucket(int capacity = 4);
    bool addBlock(const block& block);
    std::vector<block> removeAllBlocks();
    const std::vector<block>& getBlocks() const;
    bool hasSpace() const { return blocks.size() < Z; }
    size_t size() const { return blocks.size(); }
    int capacity() const { return Z; }
};

#endif