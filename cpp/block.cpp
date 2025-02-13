#include "../include/block.h"
#include "../include/encryption.h"
#include <iostream>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

block::block(int id, int leaf, const string& data, bool dummy) {
    this->id = id;
    this->leaf = leaf;
    this->data = data;
    this->dummy = dummy;
}

void block :: print_block() {
    cout << "Block ID: " << id;
    cout << ", Block Leaf: " << leaf;
    cout << ", Block Data: " << data;
    cout << ", Block Dummy: " << dummy << endl;
}
