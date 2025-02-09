#include "../include/block.h"
#include <iostream>

using namespace std;

block::block(int id, int leaf, const string& data, bool dummy) {
    this->id = id;
    this->leaf = leaf;
    this->data = data;
    this->dummy = dummy;
}

void block :: print_block() {
    cout << "Block ID: " << id << endl;
    cout << "Block Leaf: " << leaf << endl;
    cout << "Block Data: " << data << endl;
    cout << "Block Dummy: " << dummy << endl;
}