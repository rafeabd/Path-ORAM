#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include <map>
#include <random>
#include <memory>

class Server;

class Client {
private:
    std::shared_ptr<Server> server;
    std::vector<Block> stash;
    std::map<int, int> position_map;
    const int L;
    std::mt19937 rng;

    int getRandomLeaf();
    void evict(const std::vector<Block>& path);

public:
    Client(std::shared_ptr<Server> server, int num_blocks);

    // op: 0 for read, 1 for write
    Block access(int op, int id, const std::string& data = "");
};

#endif