#include "../include/bucket.h"
#include "../include/block.h"

#include <iostream>

using namespace std;

// Constructor for the Bucket class.
// Initializes a bucket with a given capacity and fills it with default blocks.
Bucket::Bucket(int capacity) : Z(capacity) {
    for (int i = 0; i < Z; i++) {
        blocks.push_back(block());  // Add default (empty) blocks to the bucket.
    }
}

// Adds a block to the first available (dummy) slot in the bucket.
// Returns true if successful, false if no empty slots are available.
bool Bucket::addBlock(const block& newBlock) {
    for (block& b : blocks) {  // Iterate over blocks in the bucket.
        if (b.dummy) {  // If a dummy block is found, replace it.
            b = newBlock;
            return true;
        }
    }
    return false;  // Return false if no dummy slots are available.
}

// Removes all blocks from the bucket and returns them as a vector.
vector<block> Bucket::removeAllBlocks() {
    vector<block> removed = blocks;  // Copy current blocks to a new vector.
    blocks.clear();  // Clear the blocks vector.
    return removed;  // Return the removed blocks.
}

// Removes a block with a specific ID from the bucket.
// Returns the removed block if found, or a default (empty) block if not found.
block Bucket::remove_block(int id) {
    for (int i = 0; i < blocks.size(); i++) {  // Iterate over the blocks.
        if (blocks[i].id == id) {  // Check if the block ID matches.
            block removed = blocks[i];  // Store the block to return.
            blocks[i] = block();  // Replace with a default block.
            return removed;  // Return the removed block.
        }
    }
    return block();  // Return a default block if no match was found.
}

// Returns a constant reference to the vector of blocks in the bucket.
const vector<block>& Bucket::getBlocks() const {
    return blocks;
}

// Prints all blocks in the bucket.
void Bucket::print_bucket() {
    for (block& b : blocks) {  // Iterate over the blocks.
        b.print_block();  // Print each block.
    }
}

// Checks if the bucket has space for more blocks.
// Returns true if there are empty slots, false otherwise.
bool Bucket::hasSpace() {
    int not_dummy = 0;  // Counter for non-dummy blocks.
    for (const block& b : blocks) {  // Iterate over the blocks.
        if (!b.dummy) {  // Count non-dummy blocks.
            not_dummy++;
        }
    }
    return not_dummy < Z;  // Return true if there is space available.
}

// Test function to check bucket and block functionality.
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
