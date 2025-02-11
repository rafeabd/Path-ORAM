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

Client::Client(int num_blocks, Server* server_ptr) : L(ceil(log2(num_blocks))), server(server_ptr) {
    key_for_id = generateEncryptionKey(32);
    key_for_data = generateEncryptionKey(32);
    
    for (int i = 0; i < num_blocks; i++) {
        position_map[i] = getRandomLeaf();
    }
}

// make sure block is on the path
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

// Compute path from leaf to root.
vector<int> Client::getPath(int leaf) {
    vector<int> path;
    int node = leaf + ((1 << L) - 1);
    while (node >= 0) {
        path.push_back(node);
        if (node == 0) break;
        node = (node - 1) / 2;
    }
    return path;
}

// server readpath
vector<block> Client::readPath(int leaf) {
    return server->give_path(leaf);
}

// lowkey eviction!
void Client::writePath(int leaf, vector<block>& stash) {
    vector<int> path = getPath(leaf);
    for (auto it = stash.begin(); it != stash.end(); ) {
        // write back for blocks in stash. dummy and normal
        if (isOnPath(it->leaf, leaf)) {
            if (server->write_block_to_path(*it, leaf)) {
                // only remove block if succesfully writeen
                it = stash.erase(it);  
                continue;
            } else {
                cout << "Block " << it->id << " could not be written." << endl;
            }
        }
        ++it;
    }
}



// op 1 is write, op 0 is read
block Client::access(int op, int id, const string& data) {
    
    // get current leaf index and find a new one
    int leaf = (position_map.count(id)) ? position_map[id] : getRandomLeaf();
    position_map[id] = getRandomLeaf();
    
    // get whole path
    vector<block> path_blocks = readPath(leaf);
    stash.insert(stash.end(), path_blocks.begin(), path_blocks.end());

    // for finding the block we want
    block result(-1, -1, "dummy", true);
    bool found = false;
    for (auto& blk : stash) {
        if (blk.id == id) {
            result = blk;
            found = true;
            if (op == 1) { 
                blk.data = data;
                blk.dummy = false;
            }
            break;
        }
    }
    // idk if this is necessary but it wouldn't run without. If a block doesn't exist,
    // it makes it and adds it.
    if (!found && op == 1) {
        block new_block(id, leaf, data, false);
        stash.push_back(new_block);
        result = new_block;
    }
    
    // highkey evict
    writePath(leaf, stash);
    
    return result;
}
