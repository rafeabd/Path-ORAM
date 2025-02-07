#include "../include/bucket.h"
#include "../include/block.h"

#include <iostream>

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

block Bucket::remove_block(int id) {
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].id == id) {
            block removed = blocks[i];
            blocks[i] = block();
            return removed;
        }
    }
    return block();
}

const vector<block>& Bucket::getBlocks() const {
    return blocks;
}

void Bucket::print_bucket() {
    // Only iterate through blocks that actually exist
    for (block& b : blocks) {
        b.print_block();
    }
}


// Test function to make sure block and bucket work togethor...seems like they do.
/*
int main() {
    block test_block(1,2,"test",false);
    block test_block_1(2,3,"test1",false);
    block test_block_2(3,4,"test2",false);
    block test_block_3(4,5,"test3",false);

    Bucket test_bucket(4);
    test_bucket.addBlock(test_block);
    test_bucket.addBlock(test_block_1);
    test_bucket.addBlock(test_block_2);
    test_bucket.addBlock(test_block_3);

    test_bucket.print_bucket();
    cout << "Removing block 2" << endl;
    test_bucket.remove_block(2);
    test_bucket.print_bucket();
    return 0;
}
*/
