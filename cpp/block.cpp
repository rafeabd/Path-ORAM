#include "../include/block.h"
#include <iostream>

using namespace std;

block::block(int id, int leaf, const std::string& data, bool dummy) {
    this->id = id;
    this->leaf = leaf;
    this->data = data;
    this->dummy = dummy;
}