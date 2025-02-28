#include "../include/block.h"
#include "../include/encryption.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

using namespace std;

block::block(int id, int leaf, const string& data, bool dummy, const vector<int>& paths) {
    this->id = id;
    this->leaf = leaf;
    this->data = data;
    this->dummy = dummy;
    this->paths = paths;
}

block::block() : id(-1), leaf(-1), data(""), dummy(true), paths(vector<int>()) {}

void block::print_block(bool print_paths) {
    cout << "Block ID: " << id;
    cout << ", Block Leaf: " << leaf;
    cout << ", Block Data: " << data;
    cout << ", Block Dummy: " << dummy << endl;
    if (print_paths && !paths.empty()) {
        cout << "Block Paths: ";
        for (int i = 0; i < paths.size(); i++) {
            cout << "R" << i << ":" << paths[i] << " ";
        }
        cout << endl;
    }
}
