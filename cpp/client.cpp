#include "../include/client.h"
#include "../include/bst.h"
#include "../include/encryption.h"
#include <openssl/rand.h>
#include <cstring>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <algorithm>

Client::Client(int num_blocks) : L(std::ceil(std::log2(num_blocks))) {
    // Initialize encryption keys.
    key_for_id = generateEncryptionKey(32);
    key_for_data = generateEncryptionKey(32);
    
    // Initialize tree.
    int tree_height = L;
    int num_buckets = (1 << (tree_height + 1)) - 1;
    tree = std::make_shared<BucketHeap>(num_buckets, 4); // Bucket capacity of 4
    
    // Initialize position map: assign each block a random leaf.
    for (int i = 0; i < num_blocks; i++) {
        position_map[i] = getRandomLeaf();
    }
}

// See if a bucket is on the proper block path
bool Client::isOnPath(int blockLeaf, int bucketIndex) {
    // get actual indices for the path. 
    std::vector<int> blockPath = getPath(blockLeaf);
    return std::find(blockPath.begin(), blockPath.end(), bucketIndex) != blockPath.end();
}

int Client::getRandomLeaf() {
    unsigned char buf[4];
    unsigned int random_value;
    std::memcpy(&random_value, buf, sizeof(random_value));
    return random_value % (1 << L);
}

// Path from leaf to root
std::vector<int> Client::getPath(int leaf) {
    std::vector<int> path;
    // Index of actual bucket based on the leaf
    int node = leaf + ((1 << L) - 1);
    while (node >= 0) {
        path.push_back(node);
        if (node == 0) {
            break;
        }
        node = (node - 1) / 2;
    }
    return path;
}

// Get all blocks 
std::vector<block> Client::readPath(int leaf) {
    std::vector<block> blocks;
    std::vector<int> path = getPath(leaf);
    for (int bucket_index : path) {
        Bucket& bucket = tree->getBucket(bucket_index);
        const std::vector<block>& bucket_blocks = bucket.getBlocks();
        blocks.insert(blocks.end(), bucket_blocks.begin(), bucket_blocks.end());
    }
    return blocks;
}

// Lowkey Eviction - starts from leaf - not secure?
void Client::writePath(int leaf, std::vector<block>& stash) {
    std::vector<int> path = getPath(leaf);
    // Reset all the buckets
    for (int bucket_index : path) {
        tree->getBucket(bucket_index) = Bucket(4); // ###important, use of bucket capacity hardcoded - should be fixed.
    }

    // placing buckets from leaf to root.
    for (int bucket_index : path) {
        for (auto it = stash.begin(); it != stash.end(); ) {
            // Don't change dummy blocks
            if (!it->dummy && isOnPath(it->leaf, bucket_index)) {
                if (tree->addBlockToBucket(bucket_index, *it)) {
                    // Bucket out of stash
                    it = stash.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

// op = 1 is a write; op = 0 is a read
block Client::access(int op, int id, const std::string& data) {

    // Encrypt block id.
    std::vector<unsigned char> id_bytes(sizeof(id));
    std::memcpy(id_bytes.data(), &id, sizeof(id));
    std::vector<unsigned char> encrypted_id = create_encrypted_id(key_for_id, id_bytes, 32);
    
    // Get old leaf and assign a new leaf to block
    int old_leaf = position_map[id];
    int new_leaf = getRandomLeaf();
    position_map[id] = new_leaf;
    
    // Get whole path
    std::vector<block> path_blocks = readPath(old_leaf);
    // Get real blocks
    for (const auto& b : path_blocks) {
        if (!b.dummy) {
            stash.push_back(b);
        }
    }
    
    // Find the actual block you want to read
    block result;
    bool found = false;
    for (auto& b : stash) {
        if (b.id == id) {
            found = true;
            // Epdate leaf of just the block you are reading
            b.leaf = new_leaf;
            //For writing if op = 1, else just read
            if (op == 1) { 
                std::vector<unsigned char> data_bytes(data.begin(), data.end());
                std::vector<unsigned char> encrypted_data = encryptData(key_for_data, data_bytes);
                b.data = std::string(encrypted_data.begin(), encrypted_data.end());
                result = b;
            } else { 
                std::vector<unsigned char> encrypted_data(b.data.begin(), b.data.end());
                std::vector<unsigned char> decrypted_data = decryptData(key_for_data, encrypted_data);
                // Decrypt
                result = b;
                result.data = std::string(decrypted_data.begin(), decrypted_data.end());
            }
            break;
        }
    }
    
    // This is incase we need to write to a block that doesn't exist at the moment.
    if (!found && op == 1) {
        std::vector<unsigned char> data_bytes(data.begin(), data.end());
        std::vector<unsigned char> encrypted_data = encryptData(key_for_data, data_bytes);
        std::string encrypted_data_str(encrypted_data.begin(), encrypted_data.end());
        
        block new_block(id, new_leaf, encrypted_data_str, false);
        stash.push_back(new_block);
        result = new_block;
    }
    
    // Writing the blocks back - old leafs.
    writePath(old_leaf, stash);
    
    return result;
}

BucketHeap* Client::getTree() {
    return tree.get();
}
