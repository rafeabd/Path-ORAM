#include "../include/bucket.h"
#include "../include/block.h"

#include <iostream>

using namespace std;

Bucket::Bucket(int capacity) : Z(capacity) {
    for (int i = 0; i < Z; i++) {
        blocks.push_back(block());
    }
}

bool Bucket::addBlock(const block& newBlock) {
    for (block& b : blocks) {
        if (b.dummy) {
            b = newBlock;
            return true;
        }
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
    // makes sure that block exists
    for (block& b : blocks) {
        b.print_block();
    }
}

bool Bucket::hasSpace() {
    int not_dummy = 0;
    for (const block& b : blocks) {
        if (!b.dummy) {
            not_dummy++;
        }
    }
    return not_dummy < Z;
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
