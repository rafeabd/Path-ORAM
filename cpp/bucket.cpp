#include "../include/bucket.h"
#include "../include/block.h"

using namespace std;

Bucket::Bucket(int capacity) : Z(capacity) {}

bool Bucket::addBlock(const block& block) {
    if (hasSpace()) {
        blocks.push_back(block);
        return true;
    }
    return false;
}

vector<block> Bucket::removeAllBlocks() {
    vector<block> removed = blocks;
    blocks.clear();
    return removed;
}

const vector<block>& Bucket::getBlocks() const {
    return blocks;
}