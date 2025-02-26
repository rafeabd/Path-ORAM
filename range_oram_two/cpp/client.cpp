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
    
    int num_trees = ceil(log2(max_range)) + 1;
    
    for (int i = 0; i < num_trees; i++) {
        // Initialize trees
        int tree_range = 1 << i;
        ORAM* tree = new ORAM(num_blocks, bucket_capacity, key, tree_range);
        oram_trees.push_back(tree);
        
        // For global counters (separate for each tree)
        evict_counter.push_back(0);

        // Initialize stashes
        unordered_map<int, block> emptyStash;
        stashes.push_back(emptyStash);
        
        // Initialize position maps
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
    
    // Create a map to track blocks we've already found by ID
    unordered_map<int, bool> found_blocks;
    for (const block& b : result) {
        found_blocks[b.id] = true;
    }
    
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
                    
                    // Check if we've already found this block
                    bool already_found = found_blocks.find(blk.id) != found_blocks.end();
                    
                    if (in_range_U && !already_found) {
                        cout << "  Adding block " << blk.id << " to result" << endl;
                        result.push_back(blk);
                        found_blocks[blk.id] = true;
                    }
                }
            }
        }
    }
    
    // Ensure the blocks we found have properly sized paths vectors
    for (block& blk : result) {
        // Resize paths vector if needed
        if (blk.paths.size() < oram_trees.size()) {
            blk.paths.resize(oram_trees.size(), -1);
        }
        
        // Make sure the path for this ORAM is set
        blk.paths[range_power] = blk.leaf;
        
        // For blocks in the range, update to the new position
        if (blk.id >= id && blk.id < id + range_size) {
            int offset = blk.id - id;
            blk.paths[range_power] = (p_prime + offset) % (1 << range_power);
            blk.leaf = blk.paths[range_power]; // Update leaf as well
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
                        
                        // Add to stash, replacing any older version with same ID
                        stash[blk.id] = blk;
                    }
                }
            }
        }
    }
    
    cout << "Stash has " << stash.size() << " blocks after reading buckets" << endl;
    
    // Step 6-11: Evict paths and write buckets back
    map<pair<int, int>, Bucket> modified_buckets;
    
    // First, organize blocks by path and level to ensure deterministic placement
    map<pair<int, int>, vector<block>> blocks_by_path_level;
    
    for (int j = h - 1; j >= 0; j--) {
        // Compute the set of path indices at level j to evict
        set<int> path_indices;
        for (int t = cnt; t < cnt + k; t++) {
            path_indices.insert(t % (1 << j));
        }
        
        cout << "Level " << j << ": Evicting to " << path_indices.size() << " buckets" << endl;
        
        // For each path index at this level
        for (int r : path_indices) {
            // Collect blocks that should go to this path at this level
            vector<block> path_blocks;
            
            // Process the stash
            auto it = stash.begin();
            while (it != stash.end()) {
                int block_id = it->first;
                block& blk = it->second;
                
                // Ensure the paths vector has enough elements
                if (blk.paths.size() <= range_power) {
                    blk.paths.resize(range_power + 1, -1);
                }
                
                // If path not set for this ORAM, assign a random path
                if (blk.paths[range_power] == -1) {
                    blk.paths[range_power] = getRandomLeaf() % (1 << range_power);
                    blk.leaf = blk.paths[range_power]; // Update leaf as well
                }
                
                // Get path label for this level
                int path_label = blk.paths[range_power] % (1 << j);
                
                if (path_label == r) {
                    cout << "  Evicting block " << block_id << " with data '" 
                         << blk.data << "' to level " << j << ", bucket " << r << endl;
                    
                    // Add to path blocks
                    path_blocks.push_back(blk);
                    it = stash.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Store path blocks
            if (!path_blocks.empty()) {
                blocks_by_path_level[{j, r}] = path_blocks;
            }
        }
    }
    
    // Create buckets from the collected blocks
    for (const auto& [path_level, blocks] : blocks_by_path_level) {
        int j = path_level.first;  // level
        int r = path_level.second; // index in level
        
        // Sort blocks by ID to ensure deterministic selection
        vector<block> sorted_blocks = blocks;
        sort(sorted_blocks.begin(), sorted_blocks.end(), 
             [](const block& a, const block& b) { return a.id < b.id; });
        
        // Select up to Z blocks for this bucket (avoid duplicates)
        vector<block> selected_blocks;
        unordered_set<int> selected_ids;
        
        for (const block& blk : sorted_blocks) {
            if (selected_ids.find(blk.id) == selected_ids.end()) {
                selected_blocks.push_back(blk);
                selected_ids.insert(blk.id);
                
                if (selected_blocks.size() >= static_cast<size_t>(bucket_capacity)) {
                    break;
                }
            }
        }
        
        // Create a new bucket with these blocks
        Bucket newBucket(bucket_capacity);
        for (const block& b : selected_blocks) {
            bool added = newBucket.addBlock(b);
            if (!added) {
                cerr << "Error: Could not add block to bucket" << endl;
            } else {
                cout << "  Added block " << b.id << " to bucket" << endl;
            }
        }
        
        // Store for later writing
        modified_buckets[{j, r}] = newBucket;
    }
    
    // Create empty buckets for paths that don't have blocks
    for (int j = h - 1; j >= 0; j--) {
        set<int> path_indices;
        for (int t = cnt; t < cnt + k; t++) {
            path_indices.insert(t % (1 << j));
        }
        
        for (int r : path_indices) {
            if (modified_buckets.find({j, r}) == modified_buckets.end()) {
                // Create an empty bucket
                Bucket emptyBucket(bucket_capacity);
                modified_buckets[{j, r}] = emptyBucket;
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
    
    // Compute a1 and a2 
    int a1 = (id / (1 << i)) * (1 << i);  
    int a2 = (a1 + (1 << i)) % num_blocks;
    
    cout << "Access: id=" << id << ", range=" << range << ", op=" << op << endl;
    cout << "Calculated a1=" << a1 << ", a2=" << a2 << endl;
    
    // The position maps for the range ORAMs (Ri) only store the start position
    // of the range, as subsequent positions are calculated using bit-reversed order
    map<int, int>& posMap = position_maps[i];
    
    // Get p1 (path for a1 in Ri) - this is our only position map lookup
    int p1;
    if (posMap.find(a1) != posMap.end()) {
        p1 = posMap[a1];
        cout << "Found position for a1=" << a1 << " in R" << i << ": " << p1 << endl;
    } else {
        p1 = getRandomLeaf() % (1 << i);
        posMap[a1] = p1;
        cout << "Assigned new position for a1=" << a1 << " in R" << i << ": " << p1 << endl;
    }
    
    // Same for a2 (if needed)
    int p2;
    if (posMap.find(a2) != posMap.end()) {
        p2 = posMap[a2];
        cout << "Found position for a2=" << a2 << " in R" << i << ": " << p2 << endl;
    } else {
        p2 = getRandomLeaf() % (1 << i);
        posMap[a2] = p2;
        cout << "Assigned new position for a2=" << a2 << " in R" << i << ": " << p2 << endl;
    }
    
    
    // Execute two ReadRange operations to get blocks
    auto [blocks1, p_prime1] = read_range(i, a1);
    auto [blocks2, p_prime2] = read_range(i, a2);
    
    // Combine results from both ReadRange operations
    vector<block> combined_blocks;
    combined_blocks.insert(combined_blocks.end(), blocks1.begin(), blocks1.end());
    combined_blocks.insert(combined_blocks.end(), blocks2.begin(), blocks2.end());
    
    cout << "Combined " << blocks1.size() << " + " << blocks2.size() 
         << " = " << combined_blocks.size() << " blocks" << endl;
    
    // Create a map of existing blocks for faster lookup
    unordered_map<int, int> block_indices;
    for (size_t idx = 0; idx < combined_blocks.size(); idx++) {
        block_indices[combined_blocks[idx].id] = idx;
    }
    
    // Update position map with new random path p_prime1 for a1
    posMap[a1] = p_prime1;
    cout << "Updated position for a1=" << a1 << " to " << p_prime1 << endl;
    
    // Similarly for a2 if needed
    posMap[a2] = p_prime2;
    cout << "Updated position for a2=" << a2 << " to " << p_prime2 << endl;
    
    
    // For write operations, create new blocks if they don't exist and update existing ones
    if (op == 1) {
        for (int blk_id = id; blk_id < id + range && blk_id < num_blocks; blk_id++) {
            auto it = block_indices.find(blk_id);
            if (it != block_indices.end()) {
                // Update existing block data
                combined_blocks[it->second].data = data;
                cout << "Write operation: updated block " << blk_id << " with data '" << data << "'" << endl;
                
                // Update the paths vector for Ri (keep other paths the same)
                if (combined_blocks[it->second].paths.size() <= i) {
                    combined_blocks[it->second].paths.resize(i + 1, -1);
                }
                
                // Assign new path for this ORAM only
                if (blk_id >= a1 && blk_id < a1 + (1 << i)) {
                    int offset = blk_id - a1;
                    // Update the path tag for Ri
                    combined_blocks[it->second].paths[i] = (p_prime1 + offset) % (1 << i);
                } else if (blk_id >= a2 && blk_id < a2 + (1 << i)) {
                    int offset = blk_id - a2;
                    // Update the path tag for Ri
                    combined_blocks[it->second].paths[i] = (p_prime2 + offset) % (1 << i);
                }
                
            } else {
                // Create a new block with paths for all ORAMs
                vector<int> paths(oram_trees.size(), -1);
                
                // Set path for ORAM i
                if (blk_id >= a1 && blk_id < a1 + (1 << i)) {
                    int offset = blk_id - a1;
                    paths[i] = (p_prime1 + offset) % (1 << i);
                } else if (blk_id >= a2 && blk_id < a2 + (1 << i)) {
                    int offset = blk_id - a2;
                    paths[i] = (p_prime2 + offset) % (1 << i);
                }
                
                // Create the block with the data
                block newBlock(blk_id, paths[i], data, false, paths);
                combined_blocks.push_back(newBlock);
                cout << "Created new block " << blk_id << " with data '" << data << "'" << endl;
                
                // Update the block_indices map
                block_indices[blk_id] = combined_blocks.size() - 1;
            }
        }
    }
    
    // For read operations, get result from the appropriate block
    if (op == 0) {
        auto it = block_indices.find(id);
        if (it != block_indices.end()) {
            result = combined_blocks[it->second].data;
            cout << "Read operation: found block " << id << " with data '" << result << "'" << endl;
        }
    }
    
    
    // Update all l+1 ORAMs using the pointer-based approach
    for (int j = 0; j <= ceil(log2(max_range)); j++) {
        // Get stash for current ORAM
        unordered_map<int, block>& stash = stashes[j];
        
        // Simply push all the read/updated blocks to this ORAM's stash
        // Reusing the path tags stored in the blocks
        for (block& blk : combined_blocks) {
            // Ensure paths vector has enough elements
            if (blk.paths.size() <= j) {
                blk.paths.resize(j + 1, -1);
            }
            
            // If this is ORAM j, update leaf to match the path
            if (j == i) {
                blk.leaf = blk.paths[j];
            }
            
            // If path not set for this ORAM, assign a random path
            if (blk.paths[j] == -1) {
                blk.paths[j] = getRandomLeaf() % (1 << ((j == 0) ? 1 : j));
                if (j == i) {
                    blk.leaf = blk.paths[j];
                }
            }
            
            // Add block to stash
            stash[blk.id] = blk;
        }
        
        cout << "Updated stash for ORAM " << j << ", now contains " << stash.size() << " blocks" << endl;
        
        // Perform batch eviction with 2^(i+1) paths as specified in the paper
        batch_evict(1 << (i+1), j);
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
    return random_value % (1 << L);
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
            for (const auto& [block_id, position] : position_maps[i]) {
                cout << "  Block " << block_id << " -> Position " << position << endl;
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
        int level_end = (1 << (level+1)) - 1;
        
        for (int i = level_start; i < level_end && i < tree->num_buckets; i++) {
            cout << "  Bucket " << i << " (physical): ";
            // Convert to normal index to access
            int normal_idx = tree->toNormalIndex(i);
            Bucket bucket = tree->read_bucket(normal_idx);
            
            // Print non-dummy blocks in this bucket
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
    
    // Initialize some test blocks with unique data
    for (int i = 0; i < 8; i++) {
        string data = "Initial block " + to_string(i) + " data";
        access(i, 1, 1, data);
        cout << "Initialized block " << i << " with '" << data << "'" << endl;
    }
    
    cout << "Test data initialization complete" << endl;
}