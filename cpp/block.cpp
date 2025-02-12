#include "../include/block.h"
#include <iostream>

using namespace std;


// Constructor definition for the `block` class.
// Initializes a block object with an ID, a leaf flag, associated data, and a dummy flag.
block::block(int id, int leaf, const string& data, bool dummy) {
    this->id = id;       // Assign the given ID to the block's ID attribute.
    this->leaf = leaf;   // Assign the given leaf flag to the block's leaf attribute.
    this->data = data;   // Assign the given data string to the block's data attribute.
    this->dummy = dummy; // Assign the given dummy flag to the block's dummy attribute.
}

// Member function to print block details.
void block :: print_block() {
// Output block details in a readable format.
    cout << "Block ID: " << id;
    cout << ", Block Leaf: " << leaf;
    cout << ", Block Data: " << data;
    cout << ", Block Dummy: " << dummy << endl;
}
