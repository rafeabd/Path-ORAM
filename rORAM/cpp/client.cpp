#include "../include/client.h"
#include "../include/oram.h"
#include "../include/encryption.h"
#include "../include/server.h"
#include "../include/helper.h"
#include <iostream>
#include <chrono>
#include <thread>
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
    this->key = generateEncryptionKey(64);
    this->num_blocks = data_to_add.size();

    //int height = ceil(log2(num_blocks + 1));
    //this->num_buckets = (1 << height) - 1;
    
    int target_buckets = ceil(num_blocks / 4.0);
    int height = ceil(log2(target_buckets + 1));
    
    // Set num_buckets to exactly 2^h-1 for a perfect full binary tree
    this->num_buckets = (1 << height) - 1;
    
    this->L = height;  // L should be the height of the tree
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
        ORAM* tree = new ORAM(num_buckets, bucket_capacity, key, tree_range, to_string(l));
        oram_trees.push_back(tree);
        //cout << "pausing for 5 seconds" << endl;
        //std::chrono::seconds dura( 5);
        //std::this_thread::sleep_for( dura );
        //cout << "pause finished" << endl;

        //cout << "adding" << endl;
        map<int,int>& position_map = position_maps[l];
        for (block& block_to_add: blocks_to_add){
            if (block_to_add.id % (1<<l) == 0){
                position_map[block_to_add.id] = block_to_add.paths[l];
            }
            block add_test_block = tree->writeBlockToPath(block_to_add, block_to_add.paths[l],key);
            if (!add_test_block.dummy){
                cerr<< "block failed to add in initialization" << endl;
                return;
            };
        }
        //cout << "done adding" << endl;
    }
}

tuple<vector<block>, int> Client::simple_read_range(int range_power, int id) {
    unordered_map<int, block> &stash = stashes[range_power];
    map<int, int> &position_map = position_maps[range_power];
    ORAM* tree = oram_trees[range_power];

    pair<int, int> range = {id, id + (1 << range_power)};
    vector<block> result;
    
    // go through stash
    try {
        vector<int> stash_keys;
        for (const auto& pair : stash) {
            stash_keys.push_back(pair.first);
        }
        
        for (int key : stash_keys) {
            if (stash.find(key) != stash.end()) {
                if (key >= range.first && key < range.second) {
                    result.push_back(stash[key]);
                }
            }
        }
    } catch (const exception& e) {
        cout << "Exception during stash iteration: " << e.what() << endl;
    } catch (...) {
        cout << "Unknown exception during stash iteration" << endl;
    }

    // Check if range exists in position_map
    int p;
    auto it = position_map.find(range.first);
    p = it->second;  
    int p_prime = getRandomLeaf();
    position_map[range.first] = p_prime;

    //cout << "reading leaf: " << p << "for range power: " << range_power << endl;
    // Read all buckets along path p
    for (int j = 0; j < L; j++) {
        try {
            //cout << "reading range at level " << j << " for path " << p << endl;
            vector<Bucket> levelBuckets = tree->try_buckets_at_level(j, p, range_power);
            for (Bucket &bucket : levelBuckets) {
                for (block &b : bucket.getBlocks()) {
                    if (!b.data.empty()) {
                        try {
                            // Decrypt the block 
                            block decrypted_b = decryptBlock(b, key);
                            //cout << "decrypted block" << endl;
                            //decrypted_b.print_block();
                            
                            // Only add non-dummy blocks in our range to the result
                            if (!decrypted_b.dummy && decrypted_b.id >= range.first && decrypted_b.id < range.second) {
                                auto it = find_if(result.begin(), result.end(), [&](const block &blk) {
                                    return blk.id == decrypted_b.id;
                                });
                                if (it == result.end()) {
                                    result.push_back(decrypted_b);
                                }
                            }
                        } catch (const exception& e) {
                            // Skip blocks that fail decryp - for debbuggin, shouldn't be necessary right now
                            // cout << "Skipping block that couldn't be decrypted: " << e.what() << endl;
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
    //for (block b: result){
    //    b.print_block();
    //}
    
    return make_tuple(result, p_prime);
}

void Client::simple_batch_evict(int eviction_number, int range_power) {
    unordered_map<int, block> &stash = stashes[range_power];
    
    // Print stash at the start of simple_batch_evict
    //cout << "simple_batch_evict: stash for range_power " << range_power << " contains:" << endl;
    //for (const auto &entry : stash) {
    //    cout << "  Block id: " << entry.first << ", data: " << entry.second.data << endl;
    //}
    
    ORAM* tree = oram_trees[range_power];
    int evict_global = evict_counter[range_power];
    // Use L directly instead of recalculating the height
    int height = this->L;

    //cout << "############################################################" << endl;
    //print_stashes();
    // Read and clear buckets on the eviction paths into stash
    for (int j = 0; j < height; j++) {
        vector<Bucket> buckets = tree->readBucketsAndClear(j, evict_global, eviction_number);
        for (Bucket &bucket : buckets) {
            for (const block &blk : bucket.getBlocks()) {
                if (!blk.data.empty()) {
                    try {
                        // Decrypt the block 
                        block decrypted_blk = decryptBlock(blk, key);
                        
                        // Only process non-dummy blocks
                        if (!decrypted_blk.dummy) {
                            // Move real block to stash if not already present
                            if (stash.find(decrypted_blk.id) == stash.end()) {
                                stash[decrypted_blk.id] = decrypted_blk;
                            }
                        }
                    } catch (const exception& e) {
                        cout << "Skipping block during eviction: " << e.what() << endl;
                    }
                }
            }
        }
    }
    //printLogicalTreeState(range_power, 10, 1);
    //print_stashes();
    //cout << "############################################################" << endl;
    // Evict from stash: place blocks into new buckets from bottom level up, level by level
    for (int j = height - 1; j >= 0; --j) {
        // Determine target bucket indices at level j for this eviction range
        set<int> target_indices;
        for (int t = evict_global; t < evict_global + eviction_number; ++t) {
            target_indices.insert(t % (1 << j));
        }
        
        // Prepare all buckets for this level before writing
        vector<pair<int, Bucket>> levelBuckets;
        
        // For each target bucket, prepare the encrypted bucket
        for (int r : target_indices) {
            Bucket newBucket(bucket_capacity);
            vector<int> candidate_ids;
            
            // Find all stash blocks whose path's prefix matches
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
            
            // Fill the bucket with candidate blocks
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
            
            // Encrypt bucket
            Bucket encryptedBucket(bucket_capacity);
            for (block &blk : newBucket.getBlocks()) {
                block encrypted_blk = encryptBlock(blk, key);
                encryptedBucket.addBlock(encrypted_blk);
            }
            
            // Add to level buckets
            levelBuckets.push_back(make_pair(r, encryptedBucket));
        }
        
        // Batch update all buckets at this level
        tree->updateBucketsAtLevel(j, levelBuckets);
    }
}

vector<block> Client::simple_access(int id, int range, int op, vector<string> data) {
    // Determine i for 2^(i-1) < range <= 2^i
    int i = -1;
    for (int c = 0; c < max_range; c++) {
        if (range > (1 << (c-1)) && range <= (1 << c)) {
            i = c;
            break;
        }
    }
    
    if (i == -1) {
        cerr << "Range " << range << " exceeds maximum supported range" << endl;
        return {};
    }
    
    // in case
    if (i >= num_trees) {
        cerr << "Calculated range power " << i << " exceeds number of trees " << num_trees << endl;
        return {};
    }
    
    int a_zero = floor(id / (1 << i)) * (1 << i);
    map<int, block> combined_read;
    vector<block> D;  

    if (op == 0) {
        D.resize(range, block());
    }
    
    //std::cout << "Before read range: a_zero=" << a_zero << ", i=" << i << ", range=" << range << std::endl;
    //std::cout << "reading range" << endl;
    // two read range
    //cout << "ranges being read: " << a_zero << ", " << a_zero + (1 << i) << " in tree " << i << endl;
    for (int a_prime : {a_zero, a_zero + (1 << i)}) {
        //std::cout << "Processing range starting at " << a_prime << std::endl;
        
        try {
            auto [blocks, p_prime] = simple_read_range(i, a_prime);
            
            // probably not necessary
            sort(blocks.begin(), blocks.end(), [](const block &a, const block &b) {
                return a.id < b.id;
            });
            
            // Update path tags for tree i
            for (block &b : blocks) {
                if (b.id >= a_prime && b.id < a_prime + (1 << i)) {
                    b.paths[i] = getRandomLeafInRange(p_prime, 1<<i);
                }
                // If reading, get data into D
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
    
    // writing update
    if (op == 1) {
        if (data.size() < range) {
            cerr << "Warning: Not enough data elements for the specified range" << endl;
        }
        
        // Update existing blocks
        map<int, int> block_index;
        vector<block> combined_blocks;
        combined_blocks.reserve(combined_read.size());
        
        for (auto &[bid, blk] : combined_read) {
            combined_blocks.push_back(blk);
            block_index[bid] = combined_blocks.size() - 1;
        }
        
        for (int j = id; j < id + range && j < id + data.size(); j++) {
            auto it = block_index.find(j);
                combined_blocks[it->second].data = data[j - id];
                combined_read[j] = combined_blocks[it->second];
        }
    }
    
    //std::cout << "Before batch evict" << endl;
    
    // evict
    for (int j = 0; j < num_trees; j++) {
        try {
            unordered_map<int, block> &stash = stashes[j];
            // remove stash blocks in range
            for (auto it = stash.begin(); it != stash.end(); ) {
                if (it->first >= a_zero && it->first < a_zero + (1 << (i+1))) {
                    it = stash.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Add/overwrite blocks from combined_read
            for (auto &[bid, blk] : combined_read) {
                stash[bid] = blk;
            }
            
            // Perform batch evict
            simple_batch_evict((1 << (i+1)), j);
            
            // Update eviction counter
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

    if (op == 0) {
        return D;
    }

    return {};  
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
    
    // Use L instead of recalculating height
    int leaf_level = L - 1;
    
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
    //cout << "new leaf:" << new_leaf << endl;
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
            int physical_index = toPhysicalIndex(normal_idx);
            Bucket bucket = tree->read_bucket(normal_idx);
            bool has_blocks = false;
            for (const block& b : bucket.getBlocks()) {
                if (!b.dummy) {
                    cout << "Block " << b.id << " ('" << b.data.substr(0, 10) << "') ";
                    for (size_t j = 0; j < b.paths.size(); j++) {
                        cout << b.paths[j] << " ";
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

void Client::printLogicalTreeState(int tree_index, int max_level, bool decrypt) {
    // Validate tree_index.
    if (tree_index < 0 || tree_index >= oram_trees.size()) {
        cout << "Invalid tree index: " << tree_index << endl;
        return;
    }
    
    ORAM* tree = oram_trees[tree_index];
    int height = ceil(log2(tree->num_buckets));
    
    cout << "===== LOGICAL TREE R" << tree_index << " STATE =====" << endl;
    
    for (int level = 0; level <= max_level && level < height; level++) {
        int level_start = (1 << level) - 1;
        int level_end = min(tree->num_buckets, (1 << (level + 1)) - 1);
        
        cout << "Level " << level << " (logical indices " << level_start << " to " << level_end - 1 << "):" << endl;
        
        for (int logical_index = level_start; logical_index < level_end; logical_index++) {
            int physical_index = toPhysicalIndex(logical_index);
            Bucket bucket = tree->read_bucket(logical_index);
            
            cout << "  Bucket " << logical_index << " (physical index " << physical_index << "): ";
            bool hasBlocks = false;
            
            for (block &b : bucket.getBlocks()) {
                if (decrypt) {
                    b = decryptBlock(b, key);
                }
                
                if (!b.dummy) {
                    cout << " Block " << b.id << " ('" << b.data.substr(0, 10) << "') ";
                    for (size_t j = 0; j < b.paths.size(); j++) {
                        cout << b.paths[j] << " ";
                    }
                    hasBlocks = true;
                }
            }
            
            if (!hasBlocks) {
                cout << "[empty]";
            }
            cout << endl;
        }
    }
    cout << endl;
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
