#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include "oram.h"
#include "server.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <tuple>

class Client {
private:
    std::vector<unsigned char> key;
    int L;  // Height of ORAM tree
    Server* server;
    int max_range;
    int num_buckets;
    int bucket_capacity;
    
    int getRandomLeaf();  // Generate a random leaf index

public:
    // Making these public for debugging
    std::vector<ORAM*> oram_trees;  // ℓ+1 ORAM trees where ℓ = ⌈log₂ L⌉
    std::vector<std::unordered_map<int, block>> stashes;  // Stash for each ORAM
    std::vector<std::map<int, int>> position_maps;  // Position map for each ORAM
    std::vector<int> evict_counter;  // Eviction counter for each ORAM
    
    Client(int num_buckets, int bucket_capacity, Server* server_ptr, int max_range);
    
    // Read a range of blocks starting at block 'id' with given range power
    std::tuple<std::vector<block>, int> read_range(int range_power, int id);
    
    // Perform batch eviction for a specific ORAM
    void batch_evict(int eviction_number, int range_power);
    
    // Access operation for reading or writing blocks
    std::string access(int id, int range, int op, std::string data);
    
    // Debug helper methods
    void print_stashes();
    void print_position_maps();
    void print_tree_state(int tree_index, int max_level = 2);
    void init_test_data();  // Initialize system with test data
};

#endif // CLIENT_H