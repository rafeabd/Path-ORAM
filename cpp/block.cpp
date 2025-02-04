#include "../include/block.h"
#include <iostream>

using namespace std;

// Initializes starting values of block to dummy values
block::block(int id, int leaf, const string& data, bool dummy) {
    this->id = id;
    this->leaf = leaf;
    this->data = data;
    this->dummy = dummy;
}