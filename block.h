#ifndef BLOCK_H
#define BLOCK_H

#include <string>

class Block {
public:
    int id;
    int leaf;
    std::string data;
    bool dummy;

    Block(int id = -1, int leaf = -1, const std::string& data = "", bool dummy = true);
};

#endif
