#include "../include/block.h"
#include "../include/encryption.h"
#include <iostream>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

/**
 * @brief Constructs a new block with given parameters
 * 
 * @param id Unique identifier for the block (-1 for dummy blocks)
 * @param leaf Target leaf position in the ORAM tree (-1 for dummy blocks)
 * @param data Data content ("dummy" for dummy blocks)
 * @param dummy Flag indicating if this is a dummy block
 * 
 * TODO for rORAM:
 * - Initialize timestamp field
 * - Set initial access count
 * - Calculate and store block size
 */
block::block(int id, int leaf, const string& data, bool dummy) {
    this->id = id;
    this->leaf = leaf;
    this->data = data;
    this->dummy = dummy;
    
    // TODO for rORAM: Add initialization for new fields
    // this->last_access = getCurrentTimestamp();
    // this->access_count = 0;
    // this->block_size = calculateBlockSize();
}

/**
 * @brief Prints block information for debugging purposes
 * 
 * Outputs all relevant block information to stdout. This is primarily
 * used for debugging and development purposes.
 * 
 * TODO for rORAM:
 * - Add printing of timestamp
 * - Include access frequency
 * - Show block size information
 */
void block::print_block() {
    cout << "Block ID: " << id;
    cout << ", Block Leaf: " << leaf;
    cout << ", Block Data: " << data;
    cout << ", Block Dummy: " << dummy << endl;
    
    // TODO for rORAM: Add printing of new fields
    // cout << ", Last Access: " << last_access;
    // cout << ", Access Count: " << access_count;
    // cout << ", Block Size: " << block_size << endl;
}

// TODO for rORAM: Add helper methods
/*
size_t block::calculateBlockSize() {
    // Calculate total size including metadata and data
    return sizeof(id) + sizeof(leaf) + data.size() + sizeof(dummy) +
           sizeof(last_access) + sizeof(access_count);
}

void block::updateAccessInfo() {
    last_access = getCurrentTimestamp();
    access_count++;
}

bool block::isHot() const {
    // Determine if block is "hot" based on access patterns
    return access_count > HOT_THRESHOLD;
}
*/
