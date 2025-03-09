#include "../include/block.h"
#include "../include/encryption.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

using namespace std;

//handles the blocks for the rOram
block::block(int id, const string& data, bool dummy, const vector<int>& paths) {
    this->id = id;
    this->data = data;
    this->dummy = dummy;
    this->paths = paths;
}

block::block() : id(-1), data(""), dummy(true), paths(vector<int>()) {}

void block::print_block() {
    cout << "Block ID: " << id;
    cout << ", Block Data: " << data;
    cout << ", Block Dummy: " << dummy << endl;
    cout << "Block Paths: ";
    for (int i = 0; i < paths.size(); i++) {
        cout << "R" << i << ":" << paths[i] << " ";
    }
    cout << endl;
}
