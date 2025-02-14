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

Client::Client(int num_blocks, Server* server_ptr, const vector<unsigned char>& encryptionKey) 
    : L(ceil(log2(num_blocks))), server(server_ptr), key(encryptionKey) {
    
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

// Reads a path from the server. The server converts leaf space to bucket space
vector<Bucket> Client::readPath(int leaf) {
    vector<Bucket> path_buckets = server->give_path(leaf);
    for (Bucket &bucket : path_buckets) {
        for (block &b : bucket.getBlocks()) {
            //cout << "Decrypting block with data length: " << b.data.size() << " and data: " << b.data << endl; 
            b = decryptBlock(b, key);
        }
    }
    return path_buckets;
}

void Client::writePath(int leaf, vector<Bucket>& path_buckets) {
    // get bucket path and ranges for the leafs they can fit to
    vector<int> global_path = getPath(leaf);
    reverse(global_path.begin(), global_path.end());
    vector<pair<int, int>> bucket_ranges;
    for (int bucketIndex : global_path) {
        int level = static_cast<int>(floor(log2(bucketIndex + 1)));
        int leftMost = (bucketIndex - ((1 << level) - 1)) << (L - level);
        int rightMost = leftMost + (1 << (L - level)) - 1;
        bucket_ranges.push_back({leftMost, rightMost});
    }
    
    // initialize each bucket with encrypted dummy blocks
    for (size_t i = 0; i < path_buckets.size(); i++) {
        Bucket newBucket(4);
        for (int j = 0; j < 4; j++) {
            block dummyBlock(-1, -1, "dummy", true);
            newBucket.addBlock(dummyBlock);
        }
        path_buckets[i] = newBucket;
    }
    
    // Place blocks from stash into the deepest bucket along the path where they fit
    for (auto it = stash.begin(); it != stash.end(); ) {
        block b = it->second;
        int assigned_index = -1;
        for (int i = static_cast<int>(global_path.size()) - 1; i >= 0; i--) {
            if (b.leaf >= bucket_ranges[i].first && b.leaf <= bucket_ranges[i].second) {
                assigned_index = i;
                break;
            }
        }
        if (assigned_index != -1 && path_buckets[assigned_index].hasSpace()) {
            path_buckets[assigned_index].addBlock(b);
            it = stash.erase(it); // remove block from stash if placed
        } else {
            ++it;
        }
    }
    
    // encrypt at the end;
    for (Bucket &bucket : path_buckets) {
        for (block &b : bucket.getBlocks()) {
            b = encryptBlock(b, key);
        }
    }
    
    // Send encrypted buckets to the server
    for (size_t i = 0; i < global_path.size(); i++) {
        //path_buckets[i].print_bucket();
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
    for (Bucket &bucket : path_buckets) {
        for (block &b : bucket.getBlocks()) {
            stash[b.id] = b;
        }
    }

    block result = block(-1, -1, "dummy", true);
    
    auto it = stash.find(id);
    if (it != stash.end()) {
        // put new leaf
        result = it->second;
        it->second.leaf = new_leaf;
        if (op == 1) { // for writing
            it->second.data = data;
            it->second.dummy = false;
        }
    } else if (op == 1) {
        // in case the id doesn't exist in current stash, make a block
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