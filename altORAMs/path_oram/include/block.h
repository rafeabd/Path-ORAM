#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"
#include <string>

using namespace std;


struct block {
    int leaf;
    int id;
    string data;
    bool dummy;

    block(int id = -1, int leaf = -1, const string& data = "dummy", bool dummy = true);
    void print_block();
};

#endif