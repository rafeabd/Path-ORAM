/*
#include "client.h"
#include "server.h"
#include <cmath>
#include <stdexcept>

using namespace std;

Client::Client(shared_ptr<Server> server, int num_blocks)
    : server(server),
    L(ceil(log2(num_blocks))),
    rng(random_device{}()) {
    if (!server) {
        throw runtime_error("Server pointer cannot be null");
    }
}

Block Client::access(int op, int id, const string& data) {
    // Get or assign leaf position
    int leaf = position_map.count(id) ? position_map[id] : getRandomLeaf();
    position_map[id] = getRandomLeaf();  // Assign new random leaf

    // Read path and update stash
    vector<Block> path_blocks = server->readPath(leaf);
    stash.insert(stash.end(), path_blocks.begin(), path_blocks.end());

    // Find and update target block
    Block result;
    for (auto& block : stash) {
        if (block.id == id && !block.dummy) {
            result = block;
            if (op == 1) {  // Write operation
                block.data = data;
            }
            break;
        }
    }

    evict(path_blocks);
    return result;
}

int Client::getRandomLeaf() {
    return uniform_int_distribution<>(0, (1 << L) - 1)(rng);
}

void Client::evict(const vector<Block>& path) {
    vector<Block> blocks_to_write;

    // Collect non-dummy blocks
    for (const Block& block : stash) {
        if (!block.dummy) {
            blocks_to_write.push_back(block);
        }
    }

    // Write path and update stash
    if (!blocks_to_write.empty()) {
        server->writePath(position_map[blocks_to_write[0].id], blocks_to_write);
    }

    stash.clear();
    for (const Block& block : blocks_to_write) {
        bool written = false;
        if (!written) {
            stash.push_back(block);
        }
    }
}
*/