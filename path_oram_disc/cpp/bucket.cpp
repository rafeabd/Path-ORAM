#include "../include/bucket.h"
#include "../include/block.h"

#include <iostream>
#include <vector>

using namespace std;

Bucket::Bucket(int capacity) : Z(capacity) {
    // Initialize the bucket with dummy blocks.
    for (int i = 0; i < Z; i++) {
        blocks.push_back(block());
    }
}

bool Bucket::addBlock(const block& newBlock) {
    if (!hasSpace()) {
        return false;
    }
    for (block& b : blocks) {
        if (b.dummy) {
            b = newBlock;
            return true;
        }
    }
    return false;
}

bool Bucket::startaddblock(block& newBlock) {
    // If we haven't reached capacity, add the block
    if (blocks.size() < Z) {
        blocks.push_back(newBlock);
        return true;
    }
    // try to replace a dummy block
    for (block &b : blocks) {
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

vector<block>& Bucket::getBlocks(){
    return blocks;
}

void Bucket::print_bucket() {
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

void Bucket::clear() {
    blocks.clear();
}