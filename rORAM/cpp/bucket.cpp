#include "../include/bucket.h"
#include "../include/block.h"
#include "../include/encryption.h"

#include <iostream>
#include <vector>

using namespace std;

Bucket::Bucket(int capacity) : Z(capacity) {
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

bool Bucket::add_block_in_tree(const block& newBlock, const vector<unsigned char>& key) {
    
    Bucket decrypted_bucket = decrypt_bucket(*this, key);
    
    if (!decrypted_bucket.addBlock(newBlock)) {
        return false;
    }
    
    *this = encrypt_bucket(decrypted_bucket, key);
    
    return true;
}

bool Bucket::startaddblock(block& newBlock) {
    if (blocks.size() < Z) {
        blocks.push_back(newBlock);
        return true;
    }
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

vector<block>& Bucket::getBlocks() {
    return blocks;
}

void Bucket::print_bucket() {
    for (block& b : blocks) {
        b.print_block();
    }
}

bool Bucket::hasSpace() {
    int realCount = 0;
    for (const block& b : blocks) {
        if (!b.dummy) {
            realCount++;
        }
    }
    return realCount < Z;
}

void Bucket::clear() {
    blocks.clear();
}