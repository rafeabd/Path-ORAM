/**
 * @file bucket.cpp
 * @brief Implements bucket functionality for Path ORAM
 */

#include "../include/bucket.h"
#include "../include/block.h"

#include <iostream>
#include <vector>

using namespace std;

/**
 * @brief Initializes a new bucket with specified capacity
 * @param capacity Maximum number of blocks the bucket can hold
 */
Bucket::Bucket(int capacity) : Z(capacity) {
    // Initialize the bucket with dummy blocks.
    for (int i = 0; i < Z; i++) {
        blocks.push_back(block());
    }
}

/**
 * @brief Adds a new block to the bucket
 * @param newBlock Block to be added
 * @return true if addition successful, false if bucket full
 */
bool Bucket::addBlock(const block& newBlock) {
    if (!hasSpace()) {
        return false;
    }
    // Replace first available dummy block
    for (block& b : blocks) {
        if (b.dummy) {
            b = newBlock;
            return true;
        }
    }
    return false;
}


/**
 * @brief Initial block addition during setup phase
 * @param newBlock Block to be added
 * @return true if addition successful, false if bucket full
 */
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

/**
 * @brief Removes all blocks from the bucket
 * @return Vector containing all removed blocks
 */
vector<block> Bucket::removeAllBlocks() {
    vector<block> removed = blocks;
    blocks.clear();
    return removed;
}

/**
 * @brief Removes a block with given integer value
 * @param int Integer value to identify the block
 * @return Removed block (dummy block if not found)
 */
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

/**
 * @brief Gets reference to stored blocks
 * @return Reference to vector of blocks
 */
vector<block>& Bucket::getBlocks(){
    return blocks;
}

/**
 * @brief Prints bucket contents for debugging
 */
void Bucket::print_bucket() {
    for (block& b : blocks) {
        b.print_block();
    }
}

/**
 * @brief Checks if bucket has space for more blocks
 * @return true if bucket can store more blocks
 */
bool Bucket::hasSpace() {
    int not_dummy = 0;
    for (const block& b : blocks) {
        if (!b.dummy) {
            not_dummy++;
        }
    }
    return not_dummy < Z;
}

/**
 * @brief Clears all blocks from bucket
 */
void Bucket::clear() {
    blocks.clear();
}
