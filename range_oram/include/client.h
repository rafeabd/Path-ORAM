#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include "bucket.h"
#include "oram.h"
#include "server.h"
#include "encryption.h"
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;

class Client {
private:

    
public:
    vector<unsigned char> key;
    int L;
    vector<int> evict_counter;
    Server* server;
    int max_range;
    int num_blocks;
    int num_buckets;
    int bucket_capacity;
    vector<ORAM*> oram_trees;
    vector<unordered_map<int, block> > stashes;
    vector<map<int,int> > position_maps;

    Client(int num_blocks, int bucket_capacity, Server* server_ptr, int max_range);
    tuple<vector<block>,int> read_range(int range_power, int leaf);
    void batch_evict(int eviction_number, int range);
    string access(int id, int range, int op, string data);
    void printRangeTree(int range);
    int getRandomLeaf();
    void print_stashes();
    void print_position_maps();
    void print_tree_state(int tree_index, int max_level);
    void init_test_data();
    void print_path(int leaf, int tree_n);
    int getRandomLeafInRange(int start, int range_size);


    
};

#endif