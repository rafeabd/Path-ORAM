#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"
#include <string>

struct block {
    int leaf;
    int id;
    std::string data;
    bool dummy;

    // Constructor declaration
    block(int id, int leaf, const std::string& data, bool dummy);
};

#endif
