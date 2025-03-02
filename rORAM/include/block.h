#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"
#include <string>
#include <vector>

using namespace std;


struct block {
    int id;
    string data;
    bool dummy;
    vector<int> paths;

    block();
    void print_block();
    block(int id, const string& data, bool dummy, const vector<int>& paths);

};

#endif