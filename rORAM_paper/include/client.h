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
    int num_trees;
    int bucket_capacity;
    vector<ORAM*> oram_trees;
    vector<unordered_map<int, block> > stashes;
    vector<map<int,int> > position_maps;

    Client(vector<pair<int,string>> data_to_add, int bucket_capacity, int max_range);
    tuple<vector<block>,int> read_range(int range_power, int leaf);
    void batch_evict(int eviction_number, int range);
    string access(int id, int range, int op, string data);
    tuple<vector<block>,int> simple_read_range(int range_power, int leaf);
    void simple_batch_evict(int eviction_number, int range);
    vector<block> simple_access(int id, int range, int op, vector<string> data);
    void printRangeTree(int range);
    int getRandomLeaf();
    void print_stashes();
    void print_position_maps();
    void print_tree_state(int tree_index, int max_level);
    void printLogicalTreeState(int tree_index, int max_level, bool decrypt);
    void init_test_data();
    void print_path(int leaf, int tree_n);
    int getRandomLeafInRange(int start, int range_size);
    void i_am_an_idiot(vector<pair<int,string>> data_to_add, int bucket_capacity, int max_range);


    
};

#endif