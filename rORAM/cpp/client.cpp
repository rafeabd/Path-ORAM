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
#include <utility>

using namespace std;

Client::Client(vector<pair<int,string>> data_to_add, int bucket_capacity, int max_range) {
    this->key = generateEncryptionKey(32);
    this->num_blocks = data_to_add.size();
    this->num_buckets = 1 << static_cast<int>(ceil(log2(num_blocks)));
    this->L = ceil(log2(this->num_buckets));
    this->max_range = max_range;
    this->bucket_capacity = bucket_capacity;
    this->num_trees = ceil(log2(max_range));

    // Initialize stashes and position maps for all trees before processing any data
    for (int l = 0; l < num_trees; l++) {
        stashes.push_back(unordered_map<int, block>());
        position_maps.push_back(map<int, int>());
        evict_counter.push_back(0);
    }

    //turn input into blocks
    vector<block> blocks_to_add;
    for (pair<int,string> data: data_to_add){
        blocks_to_add.push_back(block(data.first, data.second, false, vector<int>(num_trees, -1)));
    }

    //horrendous naming conventions
    for (int l = 0; l<num_trees;  l++){
        int current_leaf;
        for (block& block_to_add: blocks_to_add){
            if (block_to_add.id % (1<<l)==0){
                current_leaf = getRandomLeaf();
                block_to_add.paths[l] = current_leaf;
            } else {
                block_to_add.paths[l] = getRandomLeafInRange(current_leaf, 1<<l);
            }
        }
    }

    for (int l = 0; l < num_trees; l++){
        int tree_range = 1 << l;
        ORAM* tree = new ORAM(num_buckets, bucket_capacity, key, tree_range);
        oram_trees.push_back(tree);

        map<int,int>& position_map = position_maps[l];
        for (block& block_to_add: blocks_to_add){
            if (block_to_add.id % (1<<l) == 0){
                position_map[block_to_add.id] = block_to_add.paths[l];
            }
            block add_test_block = tree->writeBlockToPath(block_to_add, block_to_add.paths[l]);
            if (!add_test_block.dummy){
                cerr<< "block failed to add in initialization" << endl;
                return;
            };
        }
    }
}

tuple<vector<block>, int> Client::simple_read_range(int range_power, int id) {
    unordered_map<int, block> &stash = stashes[range_power];
    map<int, int> &position_map = position_maps[range_power];
    ORAM* tree = oram_trees[range_power];

    //cout << "acquired tree materials" << endl;
    pair<int, int> range = {id, id + (1 << range_power)};
    vector<block> result;

    //cout << "accessing stash with range [" << range.first << ", " << range.second << ")" << endl;
    //cout << "stash size: " << stash.size() << endl;
    
    // Extremely cautious stash iteration
    try {
        // Copy the keys to avoid any potential issues with concurrent modification
        vector<int> stash_keys;
        for (const auto& pair : stash) {
            stash_keys.push_back(pair.first);
        }
        
        // Iterate over the copied keys
        for (int key : stash_keys) {
            // Validate key is still in stash (in case of concurrent modification)
            if (stash.find(key) != stash.end()) {
                // Verify key is in range
                if (key >= range.first && key < range.second) {
                    // Add to result
                    result.push_back(stash[key]);
                }
            }
        }
    } catch (const exception& e) {
        cout << "Exception during stash iteration: " << e.what() << endl;
    } catch (...) {
        cout << "Unknown exception during stash iteration" << endl;
    }

    //cout << "accessing position map" << endl;
    // Check if range.first exists in position_map
    int p;
    auto it = position_map.find(range.first);
    if (it != position_map.end()) {
        p = it->second;
    } else {
        // If range.first isn't in position map, generate a new leaf position
        p = getRandomLeaf();
        position_map[range.first] = p;
    }
    
    int p_prime = getRandomLeaf();
    position_map[range.first] = p_prime;

    // Read all buckets along path p
    for (int j = 0; j < L; j++) {
        //cout << "Reading level " << j << " at leaf " << p << endl;
        try {
            vector<Bucket> levelBuckets = tree->try_buckets_at_level(j, p, range_power);
            for (Bucket &bucket : levelBuckets) {
                for (block &b : bucket.getBlocks()) {
                    if (!b.dummy && b.id >= range.first && b.id < range.second) {
                        // Add block if not already added (avoid duplicates)
                        auto it = find_if(result.begin(), result.end(), [&](const block &blk) {
                            return blk.id == b.id;
                        });
                        if (it == result.end()) {
                            result.push_back(b);
                        }
                    }
                }
            }
        } catch (const exception& e) {
            cout << "Exception reading level buckets: " << e.what() << endl;
        } catch (...) {
            cout << "Unknown exception reading level buckets" << endl;
        }
    }
    
    return make_tuple(result, p_prime);
}

void Client::simple_batch_evict(int eviction_number, int range_power) {
    unordered_map<int, block> &stash = stashes[range_power];
    ORAM* tree = oram_trees[range_power];
    int evict_global = evict_counter[range_power];
    int height = ceil(log2(tree->num_buckets));

    // Read and clear buckets on the eviction paths into stash
    //std::cout << "BATCH EVICT: reading" << endl;
    for (int j = 0; j < height; j++) {
        vector<Bucket> buckets = tree->readBucketsAndClear(j, evict_global, eviction_number);
        for (Bucket &bucket : buckets) {
            for (const block &blk : bucket.getBlocks()) {
                if (!blk.dummy) {
                    // Move real block to stash if not already present
                    if (stash.find(blk.id) == stash.end()) {
                        stash[blk.id] = blk;
                    }
                }
            }
        }
    }

    // Evict from stash: place blocks into new buckets from bottom level up
    for (int j = height - 1; j >= 0; --j) {
        // Determine target bucket indices at level j for this eviction range
        set<int> target_indices;
        for (int t = evict_global; t < evict_global + eviction_number; ++t) {
            target_indices.insert(t % (1 << j));
        }
        // For each target bucket index, fill a new bucket with as many stash blocks as possible
        //std::cout << "BATCH EVICT: filling buckets" << endl;
        for (int r : target_indices) {
            Bucket newBucket(bucket_capacity);
            vector<int> candidate_ids;
            // Find all stash blocks whose path's prefix (of length j bits) equals r
            int prefix_bits = (height - 1) - j;
            for (auto &entry : stash) {
                int block_id = entry.first;
                block &blk = entry.second;
                if (blk.dummy) continue;
                int tag = blk.paths[range_power];
                int bucket_index = (prefix_bits >= 0 ? (tag >> prefix_bits) : tag);
                if (bucket_index == r) {
                    candidate_ids.push_back(block_id);
                }
            }
            //not needed, remnant
            sort(candidate_ids.begin(), candidate_ids.end());
            for (int block_id : candidate_ids) {
                if (stash.find(block_id) == stash.end()) continue; 
                block &blk = stash[block_id];
                bool added = newBucket.addBlock(blk);
                if (added) {
                    stash.erase(block_id);
                }
                if (!newBucket.hasSpace()) {
                    break; 
                }
            }
            // Write back the new bucket if it contains any real block - should write back all buckets but need to figure out order.
            //std::cout << "BATCH EVICT: updating tree" << endl;
            tree->updateBucketAtLevel(j, r, newBucket);

        }
    }
}

vector<block> Client::simple_access(int id, int range, int op, vector<string> data) {
    // Determine i such that 2^(i-1) < range <= 2^i
    int i = -1;
    for (int c = 0; c < max_range; c++) {
        if (range > (1 << (c-1)) && range <= (1 << c)) {
            i = c;
            break;  // Add break to stop after finding correct range
        }
    }
    
    if (i == -1) {
        cerr << "Range " << range << " exceeds maximum supported range" << endl;
        return {};
    }
    
    // Sanity check on range_power
    if (i >= num_trees) {
        cerr << "Calculated range power " << i << " exceeds number of trees " << num_trees << endl;
        return {};
    }
    
    int a_zero = floor(id / (1 << i)) * (1 << i);
    map<int, block> combined_read;
    vector<block> D;  // output data blocks (for reads)

    // If reading, pre-initialize D with dummy blocks
    if (op == 0) {
        D.resize(range, block());
    }
    
    //std::cout << "Before read range: a_zero=" << a_zero << ", i=" << i << ", range=" << range << std::endl;
    
    // Perform two ReadRange operations for [a0, a0+2^i) and [a0+2^i, a0+2^(i+1))
    for (int a_prime : {a_zero, a_zero + (1 << i)}) {
        //std::cout << "Processing range starting at " << a_prime << std::endl;
        
        try {
            auto [blocks, p_prime] = simple_read_range(i, a_prime);
            
            // Sort blocks by ID for predictable ordering
            sort(blocks.begin(), blocks.end(), [](const block &a, const block &b) {
                return a.id < b.id;
            });
            
            // Update path tags for tree i (if writing, these will be new positions)
            for (block &b : blocks) {
                if (b.id >= a_prime && b.id < a_prime + (1 << i)) {
                    b.paths[i] = getRandomLeafInRange(p_prime, 1<<i);
                }
                // If reading, collect data
                if (op == 0 && b.id >= id && b.id < id + range) {
                    int idx = b.id - id;
                    if (idx >= 0 && idx < D.size()) {
                        D[idx] = b;
                    }
                }
            }
            
            // Merge blocks into combined_read map
            for (block &b : blocks) {
                combined_read[b.id] = b;
            }
        }
        catch (const exception& e) {
            cerr << "Exception during simple_read_range for a_prime=" << a_prime << ": " << e.what() << endl;
        }
        catch (...) {
            cerr << "Unknown exception during simple_read_range for a_prime=" << a_prime << endl;
        }
    }
    
    //std::cout << "Before write" << endl;
    
    // If op is WRITE (1), incorporate new data into combined_read
    if (op == 1) {
        // Check if we have enough data elements
        if (data.size() < range) {
            cerr << "Warning: Not enough data elements for the specified range" << endl;
        }
        
        // Update existing blocks and add new blocks for any IDs in [id, id+range) not present
        map<int, int> block_index;
        vector<block> combined_blocks;
        combined_blocks.reserve(combined_read.size());
        
        for (auto &[bid, blk] : combined_read) {
            combined_blocks.push_back(blk);
            block_index[bid] = combined_blocks.size() - 1;
        }
        
        for (int j = id; j < id + range && j < id + data.size(); j++) {
            auto it = block_index.find(j);
            if (it == block_index.end()) {
                // Block with this ID doesn't exist yet, create a new one with appropriate paths
                vector<int> new_paths(num_trees, -1);
                // Set path for current tree
                new_paths[i] = getRandomLeafInRange(position_maps[i][a_zero], 1<<i);
                
                block newBlock(j, data[j - id], false, new_paths);
                combined_blocks.push_back(newBlock);
                block_index[j] = combined_blocks.size() - 1;
                combined_read[j] = newBlock;
            } else {
                // Update existing block
                combined_blocks[it->second].data = data[j - id];
                combined_read[j] = combined_blocks[it->second];
            }
        }
    }
    
    //std::cout << "Before batch evict" << endl;
    
    // **Eviction phase:** remove old copies from stash and add updated blocks, then evict
    for (int j = 0; j < num_trees; j++) {
        try {
            unordered_map<int, block> &stash = stashes[j];
            // Remove any stash blocks in the updated range [a_zero, a_zero + 2^(i+1))
            for (auto it = stash.begin(); it != stash.end(); ) {
                if (it->first >= a_zero && it->first < a_zero + (1 << (i+1))) {
                    it = stash.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Add/overwrite blocks from combined_read to the stash for tree j
            for (auto &[bid, blk] : combined_read) {
                stash[bid] = blk;
            }
            
            // Perform batched eviction on tree j
            simple_batch_evict((1 << (i+1)), j);
            
            // Update eviction counter for this tree
            int total_leaves = 1 << (L - 1);
            evict_counter[j] = (evict_counter[j] + (1 << (i+1))) % total_leaves;
        }
        catch (const exception& e) {
            cerr << "Exception during eviction for tree " << j << ": " << e.what() << endl;
        }
        catch (...) {
            cerr << "Unknown exception during eviction for tree " << j << endl;
        }
    }

    // If a read operation, return the data for the requested range
    if (op == 0) {
        return D;
    }

    return {};  // for write, no direct output
}

int Client::getRandomLeaf() {
    unsigned char buf[4];
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        throw runtime_error("Failed to generate random bytes");
    }
    unsigned int random_value;
    memcpy(&random_value, buf, sizeof(random_value));
    return random_value % (1 << (L - 1));
}

int Client::getRandomLeafInRange(int start, int range_size) {
    unsigned char buf[4];
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        throw runtime_error("Failed to generate random bytes");
    }
    unsigned int random_value;
    memcpy(&random_value, buf, sizeof(random_value));
    
    // Calculate the height and leaf level
    int height = ceil(log2(num_buckets + 1)) - 1;
    int leaf_level = height - 1;
    
    // bit reverse
    int start_br = 0;
    int temp_start = start;
    for (int i = 0; i < leaf_level; i++) {
        start_br = (start_br << 1) | (temp_start & 1);
        temp_start >>= 1;
    }
    
    // Add a random offset within range_size to the bit-reversed leaf
    int new_leaf_br = (start_br + (random_value % range_size)) % (1 << leaf_level);
    
    // Bit-reverse back to get the regular leaf index
    int new_leaf = 0;
    int temp_new = new_leaf_br;
    for (int i = 0; i < leaf_level; i++) {
        new_leaf = (new_leaf << 1) | (temp_new & 1);
        temp_new >>= 1;
    }
    
    return new_leaf;
}

void Client::print_stashes() {
    cout << "===== STASH STATES =====" << endl;
    for (int i = 0; i < stashes.size(); i++) {
        cout << "Stash for ORAM R" << i << " (size: " << stashes[i].size() << "):" << endl;
        if (stashes[i].empty()) {
            cout << "  [empty]" << endl;
        } else {
            for (const auto& [block_id, blk] : stashes[i]) {
                cout << "  Block " << block_id << ": '" << blk.data << "'";
                if (!blk.paths.empty()) {
                    cout << ", paths: [";
                    for (size_t j = 0; j < blk.paths.size(); j++) {
                        cout << blk.paths[j];
                        if (j < blk.paths.size() - 1) cout << ", ";
                    }
                    cout << "]";
                }
                cout << endl;
            }
        }
        cout << endl;
    }
}

void Client::print_position_maps() {
    cout << "===== POSITION MAPS =====" << endl;
    for (int i = 0; i < position_maps.size(); i++) {
        cout << "Position Map for ORAM R" << i << " (size: " << position_maps[i].size() << "):" << endl;
        if (position_maps[i].empty()) {
            cout << "  [empty]" << endl;
        } else {
            for (const auto& [addr, leaf] : position_maps[i]) {
                cout << "  Block " << addr << " -> Leaf " << leaf << endl;
            }
        }
        cout << endl;
    }
}

void Client::print_tree_state(int tree_index, int max_level) {
    if (tree_index < 0 || tree_index >= oram_trees.size()) {
        cout << "Invalid tree index: " << tree_index << endl;
        return;
    }
    ORAM* tree = oram_trees[tree_index];
    cout << "===== TREE R" << tree_index << " STATE =====" << endl;
    for (int level = 0; level <= max_level; level++) {
        cout << "Level " << level << ":" << endl;
        int level_start = (1 << level) - 1;
        int level_end = (1 << (level + 1)) - 1;
        for (int i = level_start; i < level_end && i < tree->num_buckets; i++) {
            cout << "  Bucket " << i << " (physical): ";
            // Convert to normal index to read logically
            int normal_idx = tree->toNormalIndex(i);
            Bucket bucket = tree->read_bucket(normal_idx);
            bool has_blocks = false;
            for (const block& b : bucket.getBlocks()) {
                if (!b.dummy) {
                    cout << "Block " << b.id << " ('" << b.data.substr(0, 10) << "') ";
                    for (size_t j = 0; j < b.paths.size(); j++) {
                        cout << b.paths[j];
                    }
                    has_blocks = true;
                }
            }
            if (!has_blocks) {
                cout << "[empty]";
            }
            cout << endl;
        }
    }
}

void Client::printLogicalTreeState(int tree_index, int max_level) {
    // Validate tree_index.
    if (tree_index < 0 || tree_index >= oram_trees.size()) {
        cout << "Invalid tree index: " << tree_index << endl;
        return;
    }
    // Get the ORAM tree for this tree_index.
    ORAM* tree = oram_trees[tree_index];
    // Compute the tree height from the number of buckets.
    int height = ceil(log2(tree->num_buckets + 1));
    cout << "===== LOGICAL TREE R" << tree_index << " STATE =====" << endl;
    // Iterate over each level (up to max_level or the tree height).
    for (int level = 0; level <= max_level && level < height; level++) {
        // Logical indices at this level run from (1<<level)-1 to (1<<(level+1))-2.
        int level_start = (1 << level) - 1;
        int level_end = min(tree->num_buckets, (1 << (level + 1)) - 1);
        cout << "Level " << level << " (logical indices " << level_start 
             << " to " << level_end - 1 << "):" << endl;
        // For each logical index at this level, convert it to physical index.
        for (int logical_index = level_start; logical_index < level_end; logical_index++) {
            // Convert the logical index to its physical location.
            int physical_index = tree->toPhysicalIndex(logical_index);
            // Retrieve the bucket stored at the physical index.
            Bucket bucket = tree->heap[physical_index];
            cout << "  Bucket " << logical_index << " (physical " << physical_index << "): ";
            bool hasBlocks = false;
            // Print out each non-dummy block (e.g. showing id and a short snippet of its data).
            for (const block &b : bucket.getBlocks()) {
                if (!b.dummy) {
                    cout << "Block " << b.id << " ('" << b.data.substr(0, 10) << "') ";
                    hasBlocks = true;
                }
            }
            if (!hasBlocks) {
                cout << "[empty]";
            }
            cout << endl;
        }
        cout << endl;
    }
}

/*
void Client::init_test_data() {
    cout << "Initializing test data..." << endl;
    // Insert some initial test blocks with unique data
    for (int i = 0; i < 8; i++) {
        string data_str = "Initial block " + to_string(i) + " data";
        access(i, 1, 1, data_str);
        cout << "Initialized block " << i << " with '" << data_str << "'" << endl;
    }
    cout << "Test data initialization complete." << endl;
}
*/

void Client::print_path(int leaf, int tree_index) {
    if (tree_index < 0 || tree_index >= oram_trees.size()) {
        cout << "Invalid tree index: " << tree_index << endl;
        return;
    }
    ORAM* tree = oram_trees[tree_index];
    cout << "===== PATH TO LEAF " << leaf << " IN TREE R" << tree_index << " =====" << endl;
    for (int j = 0; j <= (L + 1); j++) {
        int level_size = 1 << j;
        int r = leaf % level_size;
        int levelStart = (1 << j) - 1;
        int physicalIndex = levelStart + r;
        int logicalIndex = tree->toNormalIndex(physicalIndex);
        cout << "Level " << j << ": Physical Bucket = " << physicalIndex 
             << ", Logical Bucket = " << logicalIndex << endl;
    }
    cout << endl;
}
