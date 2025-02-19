#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"
#include <string>
#include <vector>

using namespace std;


struct block {
    int leaf;
    int id;
    string data;
    bool dummy;
    vector<int> path;

    block(int id = -1, int leaf = -1, const string& data = "dummy", bool dummy = true);
    void print_block();
};

#endif