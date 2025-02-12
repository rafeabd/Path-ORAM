#include "../include/client.h"
#include "../include/bst.h"
#include "../include/encryption.h"
#include "../include/server.h" 
#include <iostream>
#include <openssl/rand.h>
#include <cstring>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <algorithm>

using namespace std;

// Client constructor initializes the Path ORAM client
Client::Client(int num_blocks, Server* server_ptr) 
    : L(ceil(log2(num_blocks))), server(server_ptr) {  // L represents the height of the binary tree.

    // Generate encryption keys for both block IDs and block data (used for secure storage).
    key_for_id = generateEncryptionKey(32);
    key_for_data = generateEncryptionKey(32);

    // Initialize the position map, assigning each block a random leaf node in the binary ORAM tree.
    for (int i = 0; i < num_blocks; i++) {
        position_map[i] = getRandomLeaf();
    }
}

// Checks whether a given block is located on the path from a specific leaf to the root.
bool Client::isOnPath(int blockLeaf, int bucketIndex) {
    vector<int> blockPath = getPath(blockLeaf);  // Get the path from leaf to root.
    return find(blockPath.begin(), blockPath.end(), bucketIndex) != blockPath.end();  // Check if bucketIndex is in the path.
}

// Generates a random leaf index in the binary tree.
int Client::getRandomLeaf() {
    unsigned char buf[4];
    
    // Generate 4 random bytes and store them in `buf`.
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        throw runtime_error("Failed to generate random bytes");  // Throw error if random generation fails.
    }

    unsigned int random_value;
    memcpy(&random_value, buf, sizeof(random_value));  // Convert bytes to an integer.
    return random_value % (1 << L);  // Map the random number into the range of leaf nodes.
}

// Computes the path from a given leaf node to the root in the binary tree.
vector<int> Client::getPath(int leaf) {
    vector<int> path;
    
    // Convert leaf index to its corresponding node in a complete binary tree representation.
    int node = leaf + ((1 << L) - 1);  // Convert leaf index to its position in the ORAM tree.

    // Traverse upwards from the leaf to the root.
    while (node >= 0) {
        path.push_back(node);  // Add current node to path.
        if (node == 0) break;  // Stop at the root.
        node = (node - 1) / 2;  // Move to the parent node.
    }
    return path;  // Return the computed path.
}

// Reads all blocks along the path from the given leaf to the root from the server.
vector<block> Client::readPath(int leaf) {
    return server->give_path(leaf);  // Request the path from the server.
}

// Writes back blocks from the stash to the path while ensuring eviction follows Path ORAM rules.
void Client::writePath(int leaf, vector<block>& stash) {
    vector<int> path = getPath(leaf);  // Get the path from the leaf to the root.

    // Iterate through stash and attempt to write blocks back to the path.
    for (auto it = stash.begin(); it != stash.end(); ) {
        
        // Check if the block is on the current eviction path.
        if (isOnPath(it->leaf, leaf)) {

            // Attempt to write the block to the path on the server.
            if (server->write_block_to_path(*it, leaf)) {
                // If successfully written, remove it from the stash.
                it = stash.erase(it);  
                continue;
            } else {
                // If writing fails, print an error message.
                cout << "Block " << it->id << " could not be written." << endl;
            }
        }
        ++it;  // Move to the next block.
    }
}

// Performs an oblivious access operation on the ORAM.
// `op = 1` for write, `op = 0` for read.
block Client::access(int op, int id, const string& data) {
    
    // Fetch the current leaf position of the block, or generate a new one if it doesn't exist.
    int leaf = (position_map.count(id)) ? position_map[id] : getRandomLeaf();
    position_map[id] = getRandomLeaf();  // Assign a new leaf to the block (Path ORAM invariant).

    // Retrieve all blocks along the path from the server.
    vector<block> path_blocks = readPath(leaf);
    stash.insert(stash.end(), path_blocks.begin(), path_blocks.end());  // Add them to the stash.

    // Default block to return if not found.
    block result(-1, -1, "dummy", true);
    bool found = false;

    // Search for the block in the stash.
    for (auto& blk : stash) {
        if (blk.id == id) {
            result = blk;  // Store the found block.
            found = true;

            // If writing, update the block's data and mark it as non-dummy.
            if (op == 1) { 
                blk.data = data;
                blk.dummy = false;
            }
            break;
        }
    }

    // If the block wasn't found and it's a write operation, create a new block.
    if (!found && op == 1) {
        block new_block(id, leaf, data, false);
        stash.push_back(new_block);
        result = new_block;
    }
    
    // Perform eviction by writing the path back to the server.
    writePath(leaf, stash);
    
    return result;  // Return the accessed block.
}
