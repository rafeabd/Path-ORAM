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

// Helper: Check if a bucket (identified by tree index) is on the path from a block's assigned leaf to the root.
bool Client::isOnPath(int blockLeaf, int bucketIndex) {
    // Get the path (tree indices) for the block's assigned leaf.
    std::vector<int> blockPath = getPath(blockLeaf);
    return std::find(blockPath.begin(), blockPath.end(), bucketIndex) != blockPath.end();
}

int Client::getRandomLeaf() {
    unsigned char buf[4];
    if (!RAND_bytes(buf, sizeof(buf))) {
        throw std::runtime_error("Error generating random bytes for leaf");
    }
    unsigned int random_value;
    std::memcpy(&random_value, buf, sizeof(random_value));
    return random_value % (1 << L);
}

// Get path from a leaf (number in [0, 1<<L)) to the root of the tree.
// The tree is stored as an array where leaves are offset by ((1 << L) - 1).
std::vector<int> Client::getPath(int leaf) {
    std::vector<int> path;
    // Convert the leaf number into the corresponding tree index.
    int node = leaf + ((1 << L) - 1);
    while (node >= 0) {
        path.push_back(node);
        // For a complete binary tree stored in an array,
        // the parent's index is given by (node - 1) / 2.
        if (node == 0) break; // reached root
        node = (node - 1) / 2;
    }
    return path;
}

// Read all blocks from buckets along the path from the given leaf to the root.
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

// Write blocks from the stash back into the tree along the given path.
// This function goes from the leaf upward; for each bucket, it attempts to place every block in the stash
// that is allowed (i.e. the bucket is on the path from the block's assigned leaf to the root).
// Blocks successfully placed are removed from the stash.
void Client::writePath(int leaf, std::vector<block>& stash) {
    std::vector<int> path = getPath(leaf);
    // Clear buckets along the path (resetting each bucket with the fixed capacity).
    for (int bucket_index : path) {
        tree->getBucket(bucket_index) = Bucket(4); // Assuming bucket capacity of 4
    }

    // For each bucket on the path (from the leaf upward), try to place eligible blocks.
    for (int bucket_index : path) {
        // Use an index-based loop so we can remove placed blocks from the stash.
        for (auto it = stash.begin(); it != stash.end(); ) {
            // Only attempt to place non-dummy blocks.
            if (!it->dummy && isOnPath(it->leaf, bucket_index)) {
                // Attempt to add the block to this bucket.
                if (tree->addBlockToBucket(bucket_index, *it)) {
                    // Erase the block from the stash upon successful placement.
                    it = stash.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

// Constructor: initializes the ORAM tree, keys, and position map.
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

// The Access method performs either a read or a write.
// op == 1 indicates a write; otherwise, it is a read.
block Client::access(int op, int id, const std::string& data) {
    // Encrypt the block id.
    std::vector<unsigned char> id_bytes(sizeof(id));
    std::memcpy(id_bytes.data(), &id, sizeof(id));
    std::vector<unsigned char> encrypted_id = create_encrypted_id(key_for_id, id_bytes, 32);
    
    // 1. Look up the old leaf for the block and then assign a new random leaf.
    int old_leaf = position_map[id];
    int new_leaf = getRandomLeaf();
    position_map[id] = new_leaf;
    
    // 2. Read the entire path from the old leaf into a temporary vector.
    std::vector<block> path_blocks = readPath(old_leaf);
    // Add non-dummy blocks from the path into the persistent stash.
    for (const auto& b : path_blocks) {
        if (!b.dummy) {
            stash.push_back(b);
        }
    }
    
    // 3. Search for the target block in the stash.
    block result;
    bool found = false;
    for (auto& b : stash) {
        if (b.id == id) {
            found = true;
            // Update the block's assigned leaf to the new leaf.
            b.leaf = new_leaf;
            if (op == 1) { // Write operation.
                std::vector<unsigned char> data_bytes(data.begin(), data.end());
                std::vector<unsigned char> encrypted_data = encryptData(key_for_data, data_bytes);
                b.data = std::string(encrypted_data.begin(), encrypted_data.end());
                result = b;
            } else { // Read operation.
                std::vector<unsigned char> encrypted_data(b.data.begin(), b.data.end());
                std::vector<unsigned char> decrypted_data = decryptData(key_for_data, encrypted_data);
                // Update result with the decrypted data.
                result = b;
                result.data = std::string(decrypted_data.begin(), decrypted_data.end());
            }
            break;
        }
    }
    
    // 4. If the block is not found and this is a write operation, create a new block.
    if (!found && op == 1) {
        std::vector<unsigned char> data_bytes(data.begin(), data.end());
        std::vector<unsigned char> encrypted_data = encryptData(key_for_data, data_bytes);
        std::string encrypted_data_str(encrypted_data.begin(), encrypted_data.end());
        
        block new_block(id, new_leaf, encrypted_data_str, false);
        stash.push_back(new_block);
        result = new_block;
    }
    
    // 5. Write back the blocks in the stash along the path from the old leaf to the root.
    writePath(old_leaf, stash);
    
    // Return the result (for reads, it contains the decrypted data).
    return result;
}

BucketHeap* Client::getTree() {
    return tree.get();
}
