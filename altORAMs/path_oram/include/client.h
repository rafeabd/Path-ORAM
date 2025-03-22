#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include "bucket.h"
#include "bst.h"
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
    vector<unsigned char> key;
    unordered_map<int, block> stash;
    map<int, int> position_map;
    int L;
    Server* server;  
    
    bool isOnPath(int blockLeaf, int bucketIndex);
    vector<Bucket> readPath(int leaf);
    void writePath(int leaf, vector<Bucket>& path_buckets);
    
public:
    vector<int> getPath(int leaf);
    int getRandomLeaf();
    Client(int num_blocks, Server* server_ptr, const vector<unsigned char>& encryptionKey);
    block access(int op, int id, const string& data = "");
    vector<block> range_query(int start, int end);
    void print_stash();
};

#endif