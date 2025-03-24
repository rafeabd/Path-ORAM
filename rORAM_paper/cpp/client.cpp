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

#define bucket_char_size 16384

Client::Client(vector<pair<int,string>> data_to_add, int bucket_capacity, int max_range) {
    this->key = generateEncryptionKey(64);
    this->num_blocks = data_to_add.size();

    //int height = ceil(log2(num_blocks + 1));
    //this->num_buckets = (1 << height) - 1;
    
    int target_buckets = ceil(num_blocks / 4.0);
    int height = ceil(log2(target_buckets + 1));
    this->num_buckets = (1 << height) - 1;
    
    this->L = height;  
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
                            
                            // Only add non-dummy blocks in range to the result
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
    ORAM* tree = oram_trees[range_power];
    int evict_global = evict_counter[range_power];
    int height = this->L;  

    for (int j = height-1; j >= 0; j--) {
        int levelSize = (1 << j);
        int levelStartLogical = (1 << j) - 1;

        // Determine the target logical bucket indices for eviction.
        set<int> targetLogicalIndices;
        for (int t = evict_global; t < evict_global + eviction_number; t++) {
            int offset = t % levelSize;
            int targetLogical = levelStartLogical + offset;
            targetLogicalIndices.insert(targetLogical);
        }

        // Map these logical indices to their physical indices.
        vector<int> targetPhysicalIndices;
        for (int logical : targetLogicalIndices) {
            int phys = tree->toPhysicalIndex(logical);
            targetPhysicalIndices.push_back(phys);
        }

        // Compute the minimum and maximum physical indices for one disc seek
        int minPhysical = *min_element(targetPhysicalIndices.begin(), targetPhysicalIndices.end());
        int maxPhysical = *max_element(targetPhysicalIndices.begin(), targetPhysicalIndices.end());
        int count = maxPhysical - minPhysical + 1;

        vector<Bucket> buckets = tree->read_bucket_physical_consecutive(minPhysical, count);

        // Using offset in the read buffer.
        for (int targetLogical : targetLogicalIndices) {
            int phys = tree->toPhysicalIndex(targetLogical);
            int pos = phys - minPhysical;
            if (pos < 0 || pos >= (int)buckets.size()) continue;
            Bucket &bucket = buckets[pos];
            for (const block &blk : bucket.getBlocks()) {
                if (!blk.data.empty()) {
                    try {
                        block decrypted_blk = decryptBlock(blk, key);
                        if (!decrypted_blk.dummy) {
                            if (stash.find(decrypted_blk.id) == stash.end()) {
                                stash[decrypted_blk.id] = decrypted_blk;
                            }
                        }
                    } catch (const exception &e) {
                        cout << "Skipping block during eviction read at level " << j 
                             << ": " << e.what() << endl;
                    }
                }
            }
        }

        // make buckets from the stash.
        for (int targetLogical : targetLogicalIndices) {
            int phys = tree->toPhysicalIndex(targetLogical);
            int pos = phys - minPhysical;
            if (pos < 0 || pos >= (int)buckets.size()) continue;
            Bucket newBucket(bucket_capacity);
            int prefix_bits = (height - 1) - j;
            int targetOffset = targetLogical - levelStartLogical;
            for (auto it = stash.begin(); it != stash.end(); ) {
                int tag = it->second.paths[range_power];
                int bucket_index = (prefix_bits >= 0 ? (tag >> prefix_bits) : tag);
                if (bucket_index == targetOffset) {
                    bool added = newBucket.addBlock(it->second);
                    if (added)
                        it = stash.erase(it);
                    else
                        ++it;
                    if (!newBucket.hasSpace())
                        break;
                } else {
                    ++it;
                }
            }
            // Encrypt the updated bucket.
            Bucket encryptedBucket(bucket_capacity);
            for (block &b : newBucket.getBlocks()) {
                block encrypted_blk = encryptBlock(b, key);
                encryptedBucket.addBlock(encrypted_blk);
            }
            buckets[pos] = encryptedBucket;
        }

        // Write the entire thing with one write
        string levelData;
        levelData.resize(count * bucket_char_size, ' ');
        for (int i = 0; i < count; i++) {
            string serialized = serialize_bucket(buckets[i]);
            memcpy(&levelData[i * bucket_char_size], serialized.data(), bucket_char_size);
        }
        tree->writeContiguousLevel(minPhysical, count, levelData);
    }
}


vector<block> Client::simple_access(int id, int range, int op, vector<string> data) {
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

    int leaf_level = L - 1;
    
    // bit reverse
    int start_br = 0;
    int temp_start = start;
    for (int i = 0; i < leaf_level; i++) {
        start_br = (start_br << 1) | (temp_start & 1);
        temp_start >>= 1;
    }
    
    int new_leaf_br = (start_br + (random_value % range_size)) % (1 << leaf_level);
    
    // Bit-reverse back 
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
