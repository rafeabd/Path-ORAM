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

Client::Client(int num_buckets, int bucket_capacity, Server* server_ptr, int max_range){
    this->key = generateEncryptionKey(32);
    this->L = ceil(log2(num_buckets));
    this->server = server_ptr;
    this->max_range = max_range;
    this->num_buckets = (1<<L)-1;
    this->bucket_capacity = bucket_capacity;
    
    int num_trees = ceil(log2(max_range)) + 1;
    
    for (int i = 0; i < num_trees; i++) {
        // Initialize trees
        int tree_range = 1 << i;
        ORAM* tree = new ORAM(this->num_buckets, bucket_capacity, key, tree_range);
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

// In client.cpp - read_range function

tuple<vector<block>, int> Client::read_range(int range_power, int id) {
    cout << "read_range: range_power=" << range_power << ", id=" << id << endl;
    
    // Step 1: Let U := [a, a + 2^i)
    int range_size = 1 << range_power;
    vector<int> range_U;
    for (int a = id; a < id + range_size; a++) {
        range_U.push_back(a);
    }
    
    
    // Step 2: result ← Scan stash_i for blocks in range U
    vector<block> result;
    unordered_map<int, block>& stash = stashes[range_power];
    
    for (const auto& [block_id, blk] : stash) {
        // Check if block's ID is in range U
        if (find(range_U.begin(), range_U.end(), blk.id) != range_U.end()) {
            result.push_back(blk);
        }
    }
    
    // Step 3: p ← PM_i.query(a) // Get the leaf label p for address a
    map<int, int>& pm = position_maps[range_power];
    int p;
    int p_prime = getRandomLeaf() % (1 << range_power);
    if (pm.find(id) != pm.end()) {
        p = pm[id];
    } else {
        return make_tuple(result,p_prime);
    }
    
    pm[id] = p_prime;
    
    // Create a map to track blocks we've already found by ID
    unordered_map<int, bool> found_blocks;
    for (const block& b : result) {
        found_blocks[b.id] = true;
    }
    
    // Steps 6-9: Read buckets according to the pseudocode formula
    ORAM* tree = oram_trees[range_power];
    int h = ceil(log2(tree->num_buckets+1)); // Height of the tree
    
    
    for (int j = 0; j < h; j++) {
        // Step 7: Read buckets v_j^t mod 2^j for t in [p, p+2^j]
        set<int> processed_indices; // To avoid duplicates
        vector<Bucket> level_buckets;
        
        int level_size = 1 << j;
        
        for (int t = p; t <= p + (1 << j); t++) {
            int bucket_idx = t % level_size;
            
            // Skip if already processed
            if (processed_indices.find(bucket_idx) != processed_indices.end()) {
                continue;
            }
            
            processed_indices.insert(bucket_idx);
            Bucket bucket = tree->get_bucket_at_level(j, bucket_idx);
            level_buckets.push_back(bucket);
            
        }
        
        // Steps 8-9: For valid blocks in range U, add to result if not already found
        for (Bucket& bucket : level_buckets) {
            for (block& blk : bucket.getBlocks()) {
                if (!blk.dummy) {   
                    // Check if block ID is in range U
                    bool in_range_U = find(range_U.begin(), range_U.end(), blk.id) != range_U.end();
                    
                    // Check if we've already found this block
                    bool already_found = found_blocks.find(blk.id) != found_blocks.end();
                    
                    if (in_range_U && !already_found) {
                        result.push_back(blk);
                        found_blocks[blk.id] = true;
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
    int &cnt = evict_counter[range_power];  // Counter for this specific ORAM tree
    unordered_map<int, block> &stash = stashes[range_power];


    // h = log N: the height of the ORAM tree
    int h = L;
    
    // Step 1-5: Fetch buckets from server and add valid blocks to stash
    for (int j = 0; j < h; j++) {
        int level_size = 1 << j;
        set<int> processed_indices; // To avoid duplicate bucket reads
        
        // Step 2: Read ORAM buckets V_j = {v_j^t mod 2^j : t ∈ [cnt, cnt + k)}
        for (int t = cnt; t < cnt + k; t++) {
            int bucket_idx = t % level_size;
            
            // Skip if already processed
            if (processed_indices.find(bucket_idx) != processed_indices.end()) {
                continue;
            }
            processed_indices.insert(bucket_idx);
            
            Bucket bucket = tree->get_bucket_at_level(j, bucket_idx);
            
            // Step 3-5: Process blocks in bucket
            for (const block& blk : bucket.getBlocks()) {
                if (!blk.dummy) {
                    // Step 4-5: Add to stash if no block with same address exists
                    if (stash.find(blk.id) == stash.end()) {
                        stash[blk.id] = blk;
                    }
                }
            }
        }
    }
    
    // Store blocks for each bucket to write
    map<pair<int, int>, vector<block>> bucket_blocks;
    
    // Step 6: Process from bottom-up, level-by-level
    for (int j = h - 1; j >= 0; j--) {  // Start at h-1 as 0-indexed
        int level_size = (1 << j);
        set<int> processed_paths; // Track processed paths
        
        // Step 7: For each r ∈ {t mod 2^j : t ∈ [cnt, k+cnt)}
        for (int t = cnt; t < cnt + k; t++) {
            int r = t % level_size;
            
            // Skip if already processed
            if (processed_paths.find(r) != processed_paths.end()) {
                continue;
            }
            processed_paths.insert(r);
            
            
            // Step 8: S' ← {(d,a,p0,...,pℓ) ∈ stash_i : p_i ≡ r (mod 2^j)}
            vector<block> selected_blocks;
            auto it = stash.begin();
            while (it != stash.end()) {
                block& blk = it->second;
                

                
                // Check if block belongs to this path at this level
                if (blk.paths[range_power] % (1 << j) == r) {
                    selected_blocks.push_back(blk);
                    it = stash.erase(it); // Remove from stash (Step 10)
                } else {
                    ++it;
                }
            }
            
            // Step 9: S' ← Select min(|S'|, Z) blocks from S'
            if (selected_blocks.size() > bucket_capacity) {   
                // Keep only up to Z blocks
                selected_blocks.resize(bucket_capacity);
            }
            
            // Store blocks for this bucket (Step 11: v_j^r mod 2^j ← S')
            bucket_blocks[{j, r}] = selected_blocks;
        }
    }
    
    // Steps 12-13: Write back buckets to server
    for (int j = 0; j < h; j++) {
        int level_size = (1 << j);
        set<int> processed_indices; // Track processed buckets
        
        // Step 13: Write the ORAM buckets {v_j^t mod 2^j : t ∈ [cnt, cnt + k)}
        for (int t = cnt; t < cnt + k; t++) {
            int r = t % level_size;
            
            // Skip if already processed
            if (processed_indices.find(r) != processed_indices.end()) {
                continue;
            }
            processed_indices.insert(r);
            
            // Create a new bucket
            Bucket new_bucket(bucket_capacity);
            
            // Add blocks assigned to this bucket
            if (bucket_blocks.find({j, r}) != bucket_blocks.end()) {
                for (const block& blk : bucket_blocks[{j, r}]) {
                    bool added = new_bucket.addBlock(blk);
                }
            }
            
            // Write the bucket back to the ORAM
            int levelStart = (1 << j) - 1;
            int physicalIndex = levelStart + r;
            int logicalIndex = tree->toNormalIndex(physicalIndex);
            tree->updateBucket(logicalIndex, new_bucket);

        }
    }
}

string Client::access(int id, int range, int op, string data) {
    // Step 1: Let i ∈ [0, ℓ) such that 2^(i-1) < r ≤ 2^i
    int i = 0;
    while ((1 << i) < range) {
        i++;
    }
    
    // Ensure i is within valid range
    if (i >= oram_trees.size()) {
        return "initial range is too big";
    }
    
    // Step 2: Let a0 = ⌊a/2^i⌋ · 2^i
    int a0 = (id / (1 << i)) * (1 << i);
    
    cout << "Access: id=" << id << ", range=" << range 
         << ", op=" << (op == 0 ? "read" : "write") << endl;
    cout << "Calculated i=" << i << ", a0=" << a0 << endl;
    
    // Step 3: D ← {} (Initialize empty result set)
    vector<string> D(range);
    string result = "";
    
    // Execute two ReadRange operations to get blocks
    vector<block> combined_blocks;
    map<int, int> positions; // Track positions for each range
    
    // Steps 4-7: Perform two ReadRanges to cover the range [a, a+r)
    for (int a_prime : {a0, a0 + (1 << i)}) {
        
        // Step 5: (Ba', ..., Ba'+2^i-1, p') ← Ri.ReadRange(a')
        auto [blocks, p_prime] = read_range(i, a_prime);
        
        // Store position in our tracking map
        positions[a_prime] = p_prime;
        
        // Steps 6-7: Update positions for all blocks in the range
        for (block& blk : blocks) {
            
            // If block is in the current range segment [a', a'+2^i)
            if (blk.id >= a_prime && blk.id < a_prime + (1 << i)) {
                int offset = blk.id - a_prime;
                // Ba'+j.pi ← p' + j
                blk.paths[i] = (p_prime + offset) % (1 << i);
                blk.leaf = blk.paths[i]; // Update leaf as well

            }
            
            // For read operations, collect data in range [id, id+range)
            if (op == 0 && blk.id >= id && blk.id < id + range) {
                int index = blk.id - id;
                if (index >= 0 && index < D.size()) {
                    D[index] = blk.data;
                }
            }
        }
        
        // Add blocks to combined list
        combined_blocks.insert(combined_blocks.end(), blocks.begin(), blocks.end());
    }

    
    // Update position map with new paths
    map<int, int>& posMap = position_maps[i];
    for (const auto& [a_prime, p_prime] : positions) {
        posMap[a_prime] = p_prime;
    }
    
    // Steps 8-9: Update data if writing
    if (op == 1) {
        
        // Create index for faster lookups
        map<int, int> block_indices;
        for (size_t idx = 0; idx < combined_blocks.size(); idx++) {
            block_indices[combined_blocks[idx].id] = idx;
        }
        
        // for j ∈ [a, a + r) do Bj.d ← D*
        for (int j = id; j < id + range; j++) {
            auto it = block_indices.find(j);
            
            if (it != block_indices.end()) {
                // Update existing block
                combined_blocks[it->second].data = data;
            } else {
                // Create a new block (not explicitly in pseudocode but necessary)
                vector<int> paths(oram_trees.size(), -1);
                
                // Determine which range this block belongs to
                if (j >= a0 && j < a0 + (1 << i)) {
                    int offset = j - a0;
                    paths[i] = (positions[a0] + offset) % (1 << i);
                } else if (j >= a0 + (1 << i) && j < a0 + 2 * (1 << i)) {
                    int offset = j - (a0 + (1 << i));
                    if (positions.find(a0 + (1 << i)) != positions.end()) {
                        paths[i] = (positions[a0 + (1 << i)] + offset) % (1 << i);
                    } else {
                        // Fallback if we didn't do the second ReadRange
                        paths[i] = getRandomLeaf() % (1 << i);
                    }
                }
                
                block newBlock(j, paths[i], data, false, paths);
                combined_blocks.push_back(newBlock);
            }
        }
    }
    
    // Steps 10-13: Update stashes and evict in each tree
    for (int j = 0; j <= ceil(log2(max_range)); j++) {
        
        // Step 11: stashj ← stashj \ {B ∈ stashj : a0 ≤ B.a < a0 + 2^(i+1)}
        unordered_map<int, block>& stash = stashes[j];
        auto it = stash.begin();
        int removed = 0;
        
        while (it != stash.end()) {
            if (it->first >= a0 && it->first < a0 + (1 << (i+1))) {
                it = stash.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        
        // Step 12: stashj ← stashj ∪ {Ba0, ..., Ba0+2^(i+1)−1}
        int added = 0;
        for (block& blk : combined_blocks) {
            // Check if the block is in the range [a0, a0+2^(i+1))
            if (blk.id >= a0 && blk.id < a0 + (1 << (i+1))) {
                // Ensure paths vector has enough elements
                if (blk.paths.size() <= j) {
                    blk.paths.resize(j + 1, -1);
                }
                
                // For ORAM trees other than i, we need to assign a path if not set
                if (j != i && blk.paths[j] == -1) {
                    blk.paths[j] = getRandomLeaf() % (1 << j);
                }
                
                // If this is ORAM i, ensure leaf matches the path
                if (j == i) {
                    blk.leaf = blk.paths[j];
                }
                
                // Add block to stash
                stash[blk.id] = blk;
                added++;
            }
        }
        
        // Step 13: Rj.BatchEvict(2^(i+1))
        batch_evict(1 << (i+1), j);
    }
    
    // Step 14: cnt ← cnt + 2^(i+1)
    for (int j = 0; j < evict_counter.size(); j++) {
        evict_counter[j] += (1 << (i+1));
    }
    
    // Step 15: if op = "read" then return D
    if (op == 0) {
        
        // For single block reads (simplified case not in pseudocode)
        if (range == 1) {
            if (!D[0].empty()) {
                result = D[0];
            }
        } else {
            // For range reads, combine the data from vector D
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