#include "bucket.h"

using namespace std;

Bucket::Bucket(int capacity) : Z(capacity) {}

bool Bucket::addBlock(const Block& block) {
    if (hasSpace()) {
        blocks.push_back(block);
        return true;
    }
    return false;
}

vector<Block> Bucket::removeAllBlocks() {
    vector<Block> removed = blocks;
    blocks.clear();
    return removed;
}

const vector<Block>& Bucket::getBlocks() const {
    return blocks;
}