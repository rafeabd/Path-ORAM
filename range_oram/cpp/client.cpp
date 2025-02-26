#include "../include/client.h"
#include "../include/oram.h"
#include "../include/encryption.h"
#include "../include/server.h" 
#include "../include/helper.h"
#include <iostream>
#include <openssl/rand.h>
#include <cstring>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <list>
#include <tuple>
#include <set>

using namespace std;

Client::Client(int num_blocks, int bucket_capacity, Server* server_ptr, int max_range){
    this->key = generateEncryptionKey(32);
    this->L = ceil(log2(num_blocks));
    this->server = server_ptr;
    this->max_range = max_range;
    this->num_blocks = num_blocks;
    this->bucket_capacity = bucket_capacity;
    for (int i = 0; i < ceil(log2(max_range)); i++) {
        //init trees
        int tree_range = 1 << i;
        ORAM* tree = new ORAM(num_blocks, bucket_capacity, key, tree_range);
        oram_trees.push_back(tree);
        //for global counters(seperate for each tree?)
        evict_counter.push_back(0);

        unordered_map<int, block> emptyStash;
        stashes.push_back(emptyStash);
        
        map<int, int> emptyPosMap;
        position_maps.push_back(emptyPosMap);
    }
}

tuple<vector<block>, int> Client::read_range(int range_power, int id) {
    cout << "read_range: range_power=" << range_power << ", id=" << id << endl;
    
    // Step 1: Let U := [a, a + 2^i)
    int range_size = 1 << range_power;
    vector<int> range_U;
    for (int a = id; a < id + range_size; a++) {
        if (a < num_blocks) {
            range_U.push_back(a);
        }
    }
    
    cout << "Range U contains " << range_U.size() << " blocks" << endl;
    
    // Step 2: Scan stash_i for blocks in range U
    vector<block> result;
    unordered_map<int, block>& stash = stashes[range_power];
    
    cout << "Checking stash for blocks in range (stash size: " << stash.size() << ")" << endl;
    for (const auto& [block_id, blk] : stash) {
        if (find(range_U.begin(), range_U.end(), block_id) != range_U.end()) {
            cout << "Found block " << block_id << " in stash with data '" << blk.data << "'" << endl;
            result.push_back(blk);
        }
    }
    
    // Step 3: p ← PM_i.query(a) // Get the leaf label p for address a
    map<int, int>& pm = position_maps[range_power];
    int p;
    if (pm.find(id) != pm.end()) {
        p = pm[id];
        cout << "Found position for block " << id << ": " << p << endl;
    } else {
        // If not in position map, assign a random leaf
        p = getRandomLeaf() % (1 << range_power);
        pm[id] = p;
        cout << "Assigned new position for block " << id << ": " << p << endl;
    }
    
    // Step 4: p' ← [0, N) // random leaf label p'
    int p_prime = getRandomLeaf() % (1 << range_power);
    
    // Step 5: PM_i.update(a, p') // Update the position map for address a
    pm[id] = p_prime;
    cout << "Updated position for block " << id << " to " << p_prime << endl;
    
    // Steps 6-9: Read the ORAM buckets and collect valid blocks
    ORAM* tree = oram_trees[range_power];
    int h = ceil(log2(tree->num_buckets)); // Height of the tree
    
    cout << "Reading from ORAM tree, height: " << h << endl;
    vector<int> path_indices = tree->getpathindicies_rtol(p);
    cout << "Path indices from root to leaf: ";
    for (int idx : path_indices) {
        cout << idx << " ";
    }
    cout << endl;
    
    for (int j = 0; j < h && j < path_indices.size(); j++) {
        // Step 7: Read buckets at level j
        int physical_index = path_indices[j];
        vector<Bucket> level_buckets = tree->read_level_range(j, physical_index, range_size);
        
        cout << "Level " << j << ": Reading " << level_buckets.size() << " buckets" << endl;
        
        // Step 8-9: For each valid block, check if in range U and not already in result
        for (Bucket& bucket : level_buckets) {
            for (block& blk : bucket.getBlocks()) {
                if (!blk.dummy) {
                    cout << "  Found non-dummy block " << blk.id << " with data '" 
                         << blk.data << "'" << endl;
                    
                    // Check if block ID is in range U
                    bool in_range_U = find(range_U.begin(), range_U.end(), blk.id) != range_U.end();
                    
                    // Check if block is not already in result
                    bool not_in_result = true;
                    for (const block& existing : result) {
                        if (existing.id == blk.id) {
                            not_in_result = false;
                            break;
                        }
                    }
                    
                    if (in_range_U && not_in_result) {
                        cout << "  Adding block " << blk.id << " to result" << endl;
                        result.push_back(blk);
                    }
                }
            }
        }
    }
    
    cout << "read_range returning " << result.size() << " blocks" << endl;
    
    // Step 10: return (result, p')
    return make_tuple(result, p_prime);
}

void Client::batch_evict(int eviction_number, int range_power) {
    // Ensure valid tree range
    if (range_power < 0 || range_power >= oram_trees.size()) {
        cerr << "Invalid ORAM tree range: " << range_power << endl;
        return;
    }
    
    ORAM* tree = oram_trees[range_power];
    int k = eviction_number; 
    int &cnt = evict_counter[range_power];
    unordered_map<int, block> &stash = stashes[range_power];
    
    cout << "Batch evict: range " << range_power << ", eviction count " << k << endl;
    cout << "Stash has " << stash.size() << " blocks before eviction" << endl;

    // h = log N: the height of the ORAM tree
    int h = ceil(log2(tree->num_buckets));
    
    // Step 1-5: Read buckets from tree into stash
    for (int j = 0; j < h; j++) {
        // Compute the set of bucket indices at level j to read
        set<int> bucket_indices;
        for (int t = cnt; t < cnt + k; t++) {
            bucket_indices.insert(t % (1 << j));
        }
        
        cout << "Level " << j << ": Reading " << bucket_indices.size() << " buckets" << endl;
        
        // Read each bucket and add valid blocks to stash
        for (int bucket_idx : bucket_indices) {
            // Compute the physical index of the bucket
            int levelStart = (1 << j) - 1;
            int physicalIndex = levelStart + bucket_idx;
            
            // Only process if within tree bounds
            if (physicalIndex < tree->num_buckets) {
                Bucket bucket = tree->get_bucket_at_level(j, bucket_idx);
                
                // For each valid block in the bucket
                for (const block& blk : bucket.getBlocks()) {
                    if (!blk.dummy) {
                        cout << "  Found non-dummy block " << blk.id << " with data '" 
                             << blk.data << "'" << endl;
                        
                        // Add to stash if not already present
                        if (stash.find(blk.id) == stash.end()) {
                            stash[blk.id] = blk;
                        }
                    }
                }
            }
        }
    }
    
    cout << "Stash has " << stash.size() << " blocks after reading buckets" << endl;
    
    // Step 6-11: Evict paths and write buckets back
    map<pair<int, int>, Bucket> modified_buckets;
    
    for (int j = h - 1; j >= 0; j--) {
        // Compute the set of path indices at level j to evict
        set<int> path_indices;
        for (int t = cnt; t < cnt + k; t++) {
            path_indices.insert(t % (1 << j));
        }
        
        cout << "Level " << j << ": Evicting to " << path_indices.size() << " buckets" << endl;
        
        // For each path index
        for (int r : path_indices) {
            // Select blocks for this path
            vector<block> blocks_to_evict;
            
            // Find blocks whose path label at level j matches r
            for (auto it = stash.begin(); it != stash.end(); ) {
                int block_id = it->first;
                block& blk = it->second;
                
                // Get path label for this level
                int path_label = blk.leaf % (1 << j);
                
                if (path_label == r) {
                    cout << "  Evicting block " << block_id << " with data '" 
                         << blk.data << "' to level " << j << ", bucket " << r << endl;
                    
                    blocks_to_evict.push_back(blk);
                    it = stash.erase(it);
                    
                    // Limit to bucket capacity
                    if (blocks_to_evict.size() >= static_cast<size_t>(bucket_capacity)) {
                        break;
                    }
                } else {
                    ++it;
                }
            }
            
            // Create a new bucket with these blocks
            if (!blocks_to_evict.empty()) {
                Bucket newBucket(bucket_capacity);
                for (const block& b : blocks_to_evict) {
                    bool added = newBucket.addBlock(b);
                    if (!added) {
                        cerr << "Error: Could not add block to bucket" << endl;
                    }
                }
                
                // Store for later writing
                modified_buckets[{j, r}] = newBucket;
            }
        }
    }
    
    cout << "Evicted " << modified_buckets.size() << " buckets from stash" << endl;
    cout << "Stash has " << stash.size() << " blocks after eviction" << endl;
    
    // Step 12-13: Write back buckets to the ORAM
    for (const auto& [bucket_pos, bucket] : modified_buckets) {
        int j = bucket_pos.first;  // level
        int r = bucket_pos.second; // index in level
        
        int levelStart = (1 << j) - 1;
        int physicalIndex = levelStart + r;
        
        // Convert to normal index and update
        int logicalIndex = tree->toNormalIndex(physicalIndex);
        tree->updateBucket(logicalIndex, bucket);
        
        cout << "  Updated bucket at level " << j << ", index " << r 
             << " (logical index " << logicalIndex << ")" << endl;
    }
    
    // Update the counter
    cnt = (cnt + k) % (1 << h);
    cout << "Updated eviction counter to " << cnt << endl;
}

string Client::access(int id, int range, int op, string data) {
    // Find the appropriate tree index i such that 2^i >= range
    int i = 0;
    while ((1 << i) < range) {
        i++;
    }
    
    // Ensure i is within valid range
    if (i >= oram_trees.size()) {
        cerr << "Range too large: " << range << endl;
        return "";
    }
    
    string result = "";
    
    // Get the appropriate tree and stash
    ORAM* tree = oram_trees[i];
    unordered_map<int, block>& stash = stashes[i];
    map<int, int>& posMap = position_maps[i];
    
    // For each block in the range
    for (int curr_id = id; curr_id < id + range && curr_id < num_blocks; curr_id++) {
        // Calculate the base address for position mapping
        int base_addr = (curr_id / (1 << i)) * (1 << i);
        
        // Make sure we have a position mapping
        if (posMap.find(base_addr) == posMap.end()) {
            posMap[base_addr] = getRandomLeaf() % (1 << i);
        }
        
        // Get current position and generate a new one
        int old_pos = posMap[base_addr];
        int new_pos = getRandomLeaf() % (1 << i);
        
        // Check if block is in stash
        bool found = stash.find(curr_id) != stash.end();
        
        if (!found) {
            // Read the path from the tree to the stash
            cout << "Reading path for block " << curr_id << " at leaf " << old_pos << endl;
            
            // Read the path
            vector<vector<Bucket>> path = tree->read_range(old_pos);
            
            // Search all buckets in the path for the block
            for (size_t level_idx = 0; level_idx < path.size(); level_idx++) {
                auto& level = path[level_idx];
                for (auto& bucket : level) {
                    // Check each block in the bucket
                    for (auto& blk : bucket.getBlocks()) {
                        if (!blk.dummy) {
                            cout << "    Found non-dummy block " << blk.id << " with data '" 
                                 << blk.data << "'" << endl;
                            
                            if (blk.id == curr_id) {
                                cout << "    Found target block " << curr_id << endl;
                                stash[curr_id] = blk;
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found) break;
                }
                if (found) break;
            }
        }
        
        // Process the block based on operation
        if (op == 0) { // Read
            if (found) {
                // Set the result to the block's data
                result = stash[curr_id].data;
                cout << "Setting result to: '" << result << "'" << endl;
            } else {
                cout << "Block " << curr_id << " not found in path or stash!" << endl;
            }
        } else { // Write
            if (found) {
                stash[curr_id].data = data;
                stash[curr_id].leaf = new_pos;
            } else {
                // Create a new block
                block newBlock(curr_id, new_pos, data, false);
                stash[curr_id] = newBlock;
            }
        }
        
        // Update position map
        posMap[base_addr] = new_pos;
    }
    
    // Perform 2^(i+1) evictions - the number should match the range size
    // This ensures proper security and stash bounds
    batch_evict(1 << (i+1), i);
    
    return result;
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