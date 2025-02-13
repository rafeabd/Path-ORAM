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
    
    // position map with random leafs
    for (int i = 0; i < num_blocks; i++) {
        position_map[i] = getRandomLeaf();
    }
    stash.reserve(log2(num_blocks));
}

// Don;t need anymore I think
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

// Compute path from block leaf (in leaf space) to the root - bucket indices).
vector<int> Client::getPath(int leaf) {
    vector<int> path;
    int node = leaf + ((1 << L) - 1);  // leaf space to bucket index.
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

void Client::writePath(int leaf, vector<Bucket>& path_buckets) {
    // bucket indices from leaf to root - leaf + offset to the root.
    vector<int> global_path = getPath(leaf);
    // root to leaf
    reverse(global_path.begin(), global_path.end());
    
    // find range of leafs for every bucket - this gets around using 
    //isonpath and makes it a lot faster because you don't have to check every block,
    // but it changes eviction a little so we need to check this.
    vector<pair<int, int>> bucket_ranges;
    for (int bucketIndex : global_path) {
        int level = static_cast<int>(floor(log2(bucketIndex + 1)));
        int leftMost = (bucketIndex - ((1 << level) - 1)) << (L - level);
        int rightMost = leftMost + (1 << (L - level)) - 1;
        bucket_ranges.push_back({leftMost, rightMost});
    }
    
    // reset block.
    for (Bucket &bucket : path_buckets) {
        bucket = Bucket(); 
    }
    
    //for each block, determine the deepest bucket
    //along the access path where it fits.

    // get keys.
    vector<int> keys;
    for (const auto &pair : stash) {
        keys.push_back(pair.first);
    }
    
    for (int key : keys) {
        //block in stash
        block &b = stash[key];
        int assigned_index = -1;
        
        // start from leaf
        for (int i = static_cast<int>(global_path.size()) - 1; i >= 0; i--) {
            // if leaf fits the bucket range, assign it to the bucket.
            if (b.leaf >= bucket_ranges[i].first && b.leaf <= bucket_ranges[i].second) {
                assigned_index = i;
                break;
            }
        }
        
        // try to fit block in good buckets.
        if (assigned_index != -1 && path_buckets[assigned_index].hasSpace()) {
            path_buckets[assigned_index].addBlock(b);
            stash.erase(key);
        }
    }

    //actual server interaction    
    for (size_t i = 0; i < global_path.size(); i++) {
        server->write_bucket(path_buckets[i], global_path[i]);
    }
}






// op = 1 for write, op = 0 for read.
block Client::access(int op, int id, const string& data) {
    // get current leaf and then assign a new random leaf
    int leaf = (position_map.count(id)) ? position_map[id] : getRandomLeaf();
    int new_leaf = getRandomLeaf();
    position_map[id] = new_leaf;
    
    // get buckets in path
    vector<Bucket> path_buckets = readPath(leaf);

    // update stash
    for (const Bucket &bucket : path_buckets) {
        for (const block &b : bucket.getBlocks()) {
            stash[b.id] = b;
        }
    }

    // find block we want in stash
    block result(-1, -1, "dummy", true);
    auto it = stash.find(id);
    if (it != stash.end()) {
        //put new leaf
        it->second.leaf = new_leaf;
        result = it->second;
        if (op == 1) { //for writing
            it->second.data = data;
            it->second.dummy = false;
        }
    } else if (op == 1) {
        // incase the id doesn't exist in current stash, make a block
        block new_block(id, new_leaf, data, false);
        stash.insert({id, new_block});
        result = new_block;
    }
    
    // highkey eviction
    writePath(leaf, path_buckets);
    
    return result;
}



//print stash
void Client::print_stash() {
    for (auto &pair : stash) {
        pair.second.print_block();
    }
}