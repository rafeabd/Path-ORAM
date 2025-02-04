#ifndef CLIENT_H
#define CLIENT_H

#include "bst.h"
#include "block.h"
#include <unordered_map>
#include <vector>

class Client {
    private:
        BinHeap oram;
        std::unordered_map<int, int> position_map;
        std::vector<block> stash;

    public:
        Client(int tree_height, int bucket_size);
        std::string access(int id, const std::string& data = "", bool is_write = false);
};

#endif