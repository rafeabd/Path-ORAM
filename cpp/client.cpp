#include "../include/client.h"
#include <algorithm>
#include <random>

Client::Client(int tree_height, int bucket_size) : oram(tree_height, bucket_size) {
    // Initialize the position map and stash
    position_map.clear();
    stash.clear();
}

std::string Client::access(int id, const std::string& data, bool is_write) {
    // Get the leaf node for the block
    int leaf = position_map[id];

    // Send the leaf node to the server and read the path
    std::vector<block> path = oram.readPath(leaf);

    // Add blocks from the path to the stash
    for (auto& blk : path) {
        if (!blk.dummy) {
            stash.push_back(blk);
        }
    }

    // Access the block in the stash
    std::string result;
    auto it = std::find_if(stash.begin(), stash.end(), [id](const block& blk) { return blk.id == id; });
    if (it != stash.end()) {
        result = it->data;
        if (is_write) {
            it->data = data;
        }
    } else if (is_write) {
        stash.push_back(block(id, leaf, data, false));
    }

    // Update the position map
    position_map[id] = std::rand() % (1 << oram.tree_height);

    // Evict blocks from the stash to the tree - NEED TO IMPLEMENT

    return result;
}