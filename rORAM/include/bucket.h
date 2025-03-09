#ifndef BUCKET_H
#define BUCKET_H

#include "block.h"
#include <vector>

using namespace std;

class Bucket {
private:
    vector<block> blocks;
    int height;
    int bit_reversed_order;
    int Z;

public:
    explicit Bucket(int capacity = 4);
    bool addBlock(const block& block);
    bool add_block_in_tree(const block& newBlock, const vector<unsigned char>& key);
    vector<block> removeAllBlocks();
    block remove_block(int);
    vector<block>& getBlocks();
    bool hasSpace();
    size_t size() const { return blocks.size(); }
    int capacity() const { return Z; }
    void clear(); 

    void print_bucket();
    bool startaddblock(block& newBlock);

};

#endif