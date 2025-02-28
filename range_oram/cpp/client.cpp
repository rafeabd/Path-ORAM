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

Client::Client(int num_buckets, int bucket_capacity, Server* server_ptr, int max_range) {
    this->key = generateEncryptionKey(32);
    this->L = ceil(log2(num_buckets));
    this->server = server_ptr;
    this->max_range = max_range;
    this->num_buckets = (1 << L) - 1;
    this->bucket_capacity = bucket_capacity;
    
    int num_trees = ceil(log2(max_range)) + 1;
    
    for (int i = 0; i < num_trees; i++) {
        // make trees
        int tree_range = 1 << i;
        ORAM* tree = new ORAM(this->num_buckets, bucket_capacity, key, tree_range);
        oram_trees.push_back(tree);
        
        // global counter for evicts
        evict_counter.push_back(0);

        // stashes
        unordered_map<int, block> emptyStash;
        stashes.push_back(emptyStash);
        
        // PM for all trees, sarting with random leafs all around.
        map<int, int> emptyPosMap;
        for (int j = 0; j * tree_range <= num_buckets; j++) {
            int address = j * tree_range;
            emptyPosMap[address] = getRandomLeaf();
        }
        position_maps.push_back(emptyPosMap);
    }
}

tuple<vector<block>, int> Client::read_range(int range_power, int id) {
    // line 1, U := [a, a + 2^i)
    int range_size = 1 << range_power;
    vector<int> range_U;
    for (int a = id; a < id + range_size; a++) {
        range_U.push_back(a);
    }
    
    // line 2, result ← stash_i for blocks in U
    vector<block> result;
    unordered_map<int, block>& stash = stashes[range_power];
    for (const auto& [block_id, blk] : stash) {
        if (find(range_U.begin(), range_U.end(), blk.id) != range_U.end()) {
            result.push_back(blk);
        }
    }
    
    // line 3, get PM position and get new leaf IF you are writing - I think
    map<int, int>& pm = position_maps[range_power];
    int p;
    if (pm.find(id) != pm.end()) {
        p = pm[id];
    } else {
        p = getRandomLeaf();
    }
    
    // line 4: newleaf p' for a
    int p_prime = getRandomLeaf();
    pm[id] = p_prime;
    
    // this was for duplication problems, you track what you find.
    unordered_map<int, bool> found_blocks;
    for (const block& b : result) {
        found_blocks[b.id] = true;
    }
    
    // line 6-9, read the buckets in each level along the path
    ORAM* tree = oram_trees[range_power];
    int h = ceil(log2(tree->num_buckets + 1)); 
    for (int j = 0; j < h; j++) {
        // line 7, all buckets at level j for range U.
        vector<Bucket> level_buckets = tree->readBucketsAtLevel(j, p, range_size);
        
        // line 8, get blocks that arn't duplicates... again this is redundant but at some point
        // I guess I found it necessary
        for (Bucket& bucket : level_buckets) {
            for (block& blk : bucket.getBlocks()) {
                if (!blk.dummy) {
                    bool in_range = (find(range_U.begin(), range_U.end(), blk.id) != range_U.end());
                    bool already_found = (found_blocks.find(blk.id) != found_blocks.end());
                    if (in_range && !already_found) {
                        result.push_back(blk);
                        found_blocks[blk.id] = true;
                    }
                }
            }
        }
    }
    
    // line 9: return blocks and updated leaf
    return make_tuple(result, p_prime);
}

void Client::batch_evict(int eviction_number, int range_power) {
    if (range_power < 0 || range_power >= oram_trees.size()) {
        cerr << "Invalid ORAM tree range: " << range_power << endl;
        return;
    }
    
    ORAM* tree = oram_trees[range_power];
    int k = eviction_number; 
    int &cnt = evict_counter[range_power];
    unordered_map<int, block> &stash = stashes[range_power];
    
    //cout << "Batch evict: range " << range_power << ", eviction count " << k << endl;
    //cout << "Stash has " << stash.size() << " blocks before eviction" << endl;
    
    // just for when I was losing and adding blocks, not important or part of psuedo code.
    set<int> before_ids;
    for (auto& [bid, blk] : stash) {
        if (!blk.dummy) before_ids.insert(bid);
    }
    for (int idx = 0; idx < tree->num_buckets; ++idx) {
        Bucket b = tree->read_bucket(idx);
        for (const block& blk : b.getBlocks()) {
            if (!blk.dummy) {
                before_ids.insert(blk.id);
            }
        }
    }
    int before_count = before_ids.size();
    //cout << "total unique real blocks before eviction: " << before_count << endl;
    
    // line 1-5, get k blocks for each level, for path cnt.
    int height = ceil(log2(tree->num_buckets + 1));  // tree height, a little unclear between +1 or not.
    
    for (int j = 0; j < height; j++) {
        vector<Bucket> buckets = tree->readBucketsAndClear(j, cnt, k);
        for (Bucket& bucket : buckets) {
            for (const block& blk : bucket.getBlocks()) {
                if (!blk.dummy) {
                    if (stash.find(blk.id) == stash.end()) {
                        stash[blk.id] = blk;  // add to stash if not already there, little different from paper I think, but shouldn't
                                                // affect it?
                    }
                }
            }
        }
    }
    
    
    //cout << "Stash has " << stash.size() << " blocks after reading buckets" << endl;
    
    // line 6-11, put blocks from stash into a "usable blocks" in bottom-up
    map<pair<int, int>, Bucket> modified_buckets;
    vector<int> successfully_evicted_blocks;
    
    for (int j = height - 1; j >= 0; j--) {
        // target bucket indices at level j for eviction paths
        set<int> path_indices;
        for (int t = cnt; t < cnt + k; t++) {
            path_indices.insert(t % (1 << j));
        }
        
        // for r bucket, prepare a new bucket...also pretty poor implementation 
        for (int r : path_indices) {
            Bucket newBucket(bucket_capacity); 
            
            // usable blocks which match bucket path
            vector<int> candidate_block_ids;
            for (auto& [block_id, blk] : stash) {
                // check this - its suspicious. It uses the prefix bits to match bucket/block paths, which should work in theory
                // but I probably did it wrong.
                int path_label = blk.paths[range_power];
                int prefix_bits = (height - 1) - j;
                int target_bucket = (prefix_bits >= 0 ? (path_label >> prefix_bits) : path_label);
                if (target_bucket == r) {
                    candidate_block_ids.push_back(block_id);
                }
            }
            sort(candidate_block_ids.begin(), candidate_block_ids.end());  // not necessary probably
            
            // add block to bucket
            for (int block_id : candidate_block_ids) {
                if (find(successfully_evicted_blocks.begin(), successfully_evicted_blocks.end(), block_id) 
                        != successfully_evicted_blocks.end()) {
                    continue;
                }
                block& blk = stash[block_id];
                bool added = newBucket.addBlock(blk);
                if (added) {
                    successfully_evicted_blocks.push_back(block_id);
                    stash.erase(block_id);
                }
                // Stop adding if bucket is full
                if (!newBucket.hasSpace()) {
                    break;
                }
            }
            
            // if new, write back. SHould probably write back all of them.
            bool hasRealBlock = false;
            for (const block& b : newBucket.getBlocks()) {
                if (!b.dummy) { hasRealBlock = true; break; }
            }
            if (hasRealBlock) {
                modified_buckets[{j, r}] = newBucket;
            }
        }
    }
    
    // line 12-13, writing back changed buckets. - leaks info so should change to all buckets.
    for (auto& [bucket_pos, bucket] : modified_buckets) {
        int j = bucket_pos.first;   // level
        int r = bucket_pos.second;  // index in level
        try {
            tree->updateBucketAtLevel(j, r, bucket);
            //cout << "  Updated bucket at level " << j << ", index " << r << endl;
        } catch (const exception& e) {
            // back to stash if failed
            cerr << "Error updating bucket at level " << j << ", index " << r << ": " << e.what() << endl;
            for (const block& blk : bucket.getBlocks()) {
                if (!blk.dummy) {
                    stash[blk.id] = blk;
                    auto it = find(successfully_evicted_blocks.begin(), successfully_evicted_blocks.end(), blk.id);
                    if (it != successfully_evicted_blocks.end()) {
                        successfully_evicted_blocks.erase(it);
                    }
                }
            }
        }
    }
    
    // evict counter update.
    cnt = (cnt + k) % (1 << (height - 1));
    //cout << "Updated eviction counter to " << cnt << endl;
}

string Client::access(int id, int range, int op, string data) {
    // Determine i such that 2^i >= range (range power)
    int i = 0;
    while ((1 << i) < range) {
        i++;
    }
    if (i >= oram_trees.size()) {
        cerr << "Range " << range << " exceeds maximum supported range." << endl;
        return "";
    }
    // Calculate base address a0 for the range [id, id+range)
    int a0 = (id / (1 << i)) * (1 << i);
    
    //line 1
    vector<string> D(range);
    string result = "";
    
    // Perform two ReadRange operations for a0 and a0 + 2^i, lines 4-5.
    vector<block> combined_blocks;
    map<int, int> positions;
    for (int a_prime : {a0, a0 + (1 << i)}) {
        // Execute readrange for segment starting at a_prime
        auto [blocks, p_prime] = read_range(i, a_prime);
        positions[a_prime] = p_prime;
        // Update each block's path for tree i (line 7)
        for (block& blk : blocks) {
            if (blk.id >= a_prime && blk.id < a_prime + (1 << i)) {
                int offset = blk.id - a_prime;
                blk.paths[i] = p_prime + offset;     // assign new path for this tree
                blk.leaf = blk.paths[i];            
            }
            // If read, get data
            if (op == 0 && blk.id >= id && blk.id < id + range) {
                int idx = blk.id - id;
                if (idx >= 0 && idx < D.size()) {
                    D[idx] = blk.data;
                }
            }
        }
        // combine both reads
        combined_blocks.insert(combined_blocks.end(), blocks.begin(), blocks.end());
    }
    //update internal position map.
    map<int, int>& posMap = position_maps[i];
    for (const auto& [base_addr, new_leaf] : positions) {
        posMap[base_addr] = new_leaf;
    }
    
    // for write, write new data (lines 8-9)
    if (op == 1) {
        // lookup existing blocks
        map<int, int> block_index;
        for (size_t idx = 0; idx < combined_blocks.size(); idx++) {
            block_index[combined_blocks[idx].id] = idx;
        }
        // for [id, id+range), update OR CREATE!?
        for (int j = id; j < id + range; j++) {
            auto it = block_index.find(j);
            if (it != block_index.end()) {
                // update data in an existing block
                combined_blocks[it->second].data = data;
            } else {
                // create a new block for this address
                vector<int> paths(oram_trees.size(), -1);
                // set path for current tree i using the base new pos
                int base_i = (j / (1 << i)) * (1 << i);
                int offset = j - base_i;
                if (positions.find(base_i) != positions.end()) {
                    paths[i] = positions[base_i] + offset;
                } else {
                    paths[i] = getRandomLeaf();
                }
                // For other trees, assign random paths?!
                for (int k = 0; k < oram_trees.size(); k++) {
                    if (k != i) {
                        paths[k] = getRandomLeaf();
                    }
                }
                block newBlock(j, paths[i], data, false, paths);
                combined_blocks.push_back(newBlock);
            }
        }
    }
    
    // i was struggling with blocks dissapearing, so this tracks all the blocks to make sure they
    // are implemented correctly
    set<int> all_block_ids;
    // all ID from combined blocks
    for (const block& blk : combined_blocks) {
        all_block_ids.insert(blk.id);
    }
    // Include all ids in all stashes?
    for (int j = 0; j < stashes.size(); j++) {
        for (const auto& [block_id, _] : stashes[j]) {
            all_block_ids.insert(block_id);
        }
    }
    // this is slow, and probably not necessary.
    map<int, block> all_blocks;
    for (const block& blk : combined_blocks) {
        all_blocks[blk.id] = blk;
    }
    for (int j = 0; j < stashes.size(); j++) {
        for (const auto& [block_id, blk] : stashes[j]) {
            if (all_blocks.find(block_id) == all_blocks.end()) {
                all_blocks[block_id] = blk;
            }
        }
    }
    
    // lines 10-13: update stashes and evict
    for (int j = 0; j < oram_trees.size(); j++) {
        unordered_map<int, block>& stash = stashes[j];
        // line 11: Remove any blocks in the range [a0, a0 + 2^(i+1)) the stash
        for (auto it = stash.begin(); it != stash.end();) {
            if (it->first >= a0 && it->first < a0 + (1 << (i+1))) {
                it = stash.erase(it);
            } else {
                ++it;
            }
        }
        // line 12: Add  combined_blocks that are in [a0, a0 + 2^(i+1)) to stash
        for (block& blk : combined_blocks) {
            if (blk.id >= a0 && blk.id < a0 + (1 << (i+1))) {
                stash[blk.id] = blk;
            }
        }
        // also not needed, but I had to do it at somepoint:
        // make sure all blocks outside the range are in stash
        for (auto& [block_id, blk] : all_blocks) {
            if (block_id >= a0 && block_id < a0 + (1 << (i+1))) {
                continue;  
            }
            if (stash.find(block_id) == stash.end()) {
                block new_blk = blk;
                if (j >= new_blk.paths.size() || new_blk.paths[j] == -1) {
                    // also shouldn't technically be necessary, but I had seg fault errors.
                    if (j >= new_blk.paths.size()) {
                        new_blk.paths.resize(oram_trees.size(), -1);
                    }
                    new_blk.paths[j] = getRandomLeaf();
                }
                stash[block_id] = new_blk;
            }
        }
        // evict for each tree.
        batch_evict(1 << (i+1), j);
    }    
    // read return data
    if (op == 0) {
        if (range == 1) {
            if (!D[0].empty()) {
                result = D[0];
            }
        } else {
            for (int j = 0; j < range; j++) {
                result += D[j];
            }
        }
    }
    return result;
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

// Debug helper methods

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
