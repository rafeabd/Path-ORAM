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
#include <unordered_set>

using namespace std;

Client::Client(int num_blocks, Server* server_ptr) : L(ceil(log2(num_blocks))), server(server_ptr) {
    key_for_id = generateEncryptionKey(32);
    key_for_data = generateEncryptionKey(32);
    
    // Initialize the position map with random leaves in leaf space ([0, (1<<L)-1]).
    for (int i = 0; i < num_blocks; i++) {
        position_map[i] = getRandomLeaf();
    }
    stash.reserve(log2(num_blocks));
}

// Check if a block’s assigned leaf is on the current access path.
// Note: both parameters are in leaf space. getPath converts a leaf to bucket indices.
bool Client::isOnPath(int blockLeaf, int bucketIndex) {
    vector<int> blockPath = getPath(blockLeaf);
    return find(blockPath.begin(), blockPath.end(), bucketIndex) != blockPath.end();
}

int Client::getRandomLeaf() {
    unsigned char buf[4];
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        throw runtime_error("Failed to generate random bytes");
    }
    unsigned int random_value;
    memcpy(&random_value, buf, sizeof(random_value));
    return random_value % (1 << L);
}

// Compute the path from the block's leaf (in leaf space) to the root (bucket indices).
vector<int> Client::getPath(int leaf) {
    vector<int> path;
    int node = leaf + ((1 << L) - 1);  // Convert to bucket index.
    while (node >= 0) {
        path.push_back(node);
        if (node == 0) break;
        node = (node - 1) / 2;
    }
    return path;
}

// Reads a path from the server. The server converts leaf space to bucket space.
vector<Bucket> Client::readPath(int leaf) {
    return server->give_path(leaf);
}

void Client::writePath(int leaf, vector<block>& stash, vector<Bucket>& path_buckets) {
    // Compute global path indices (from root to leaf).
    vector<int> global_path = getPath(leaf);
    reverse(global_path.begin(), global_path.end());
    
    if (global_path.size() != path_buckets.size()) {
        throw runtime_error("Mismatch between global path and path_buckets lengths");
    }
    
    // For each bucket in the path:
    for (size_t i = 0; i < path_buckets.size(); i++) {
        Bucket &bucket = path_buckets[i];
        
        // Clear the bucket to remove any stale data.
        bucket = Bucket();  // Z is the bucket capacity.
        
        // Try to fill the bucket from the stash with eligible blocks.
        for (auto it = stash.begin(); it != stash.end(); ) {
            if (isOnPath(it->leaf, global_path[i]) && bucket.hasSpace()) {
                bucket.addBlock(*it);
                it = stash.erase(it);
            } else {
                ++it;
            }
        }
        
        // Write this fresh bucket to the server.
        server->write_bucket(bucket, global_path[i]);
    }
}



// op = 1 for write, op = 0 for read.
block Client::access(int op, int id, const string& data) {
    // get leaf.
    int leaf = (position_map.count(id)) ? position_map[id] : getRandomLeaf(); //need to check
    // new random leaf
    int new_leaf = getRandomLeaf();
    position_map[id] = new_leaf;
    
    // get path in the form of buckets
    vector<Bucket> path_buckets = readPath(leaf);

    //adding buckets to stash    
    for (const Bucket &bucket : path_buckets) {
        // If a block with the same id is already in the stash, update it; else insert.
        for (const block &b : bucket.getBlocks()) {
            bool found = false;
            for (block &s : stash) {
                if (s.id == b.id) {
                    s = b;  // Update to the latest version.
                    found = true;
                    break;
                }
            }
            if (!found){
                stash.push_back(b);
            }
        }
    }

    // Try to find the block we want.
    block result(-1, -1, "dummy", true);
    bool found = false;
    for (auto &blk : stash) {
        if (blk.id == id) {
            blk.leaf = new_leaf;
            result = blk;
            found = true;
            if (op == 1) { // For write operations, update the block.
                blk.data = data;
                blk.dummy = false;
            }
            break;
        }
    }
    // If the block was not found and this is a write, create a new block.
    if (!found && op == 1) {
        block new_block(id, new_leaf, data, false);
        stash.push_back(new_block);
        result = new_block;
    }
    
    // Attempt to evict eligible blocks along the path.
    writePath(leaf, stash, path_buckets);
    
    return result;
}


//print stash
void Client::print_stash(){
    for (block b : stash){
        b.print_block();
    }
}