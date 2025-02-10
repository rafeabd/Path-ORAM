#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include "bucket.h"
#include "bst.h"
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;

class Client {
private:
    vector<unsigned char> key_for_id;
    vector<unsigned char> key_for_data;
    vector<block> stash;
    map<int, int> position_map;
    int L;
    shared_ptr<BucketHeap> tree;  // The oram

    
    bool isOnPath(int, int);
    vector<int> getPath(int leaf);
    vector<block> readPath(int leaf);
    void writePath(int leaf, vector<block>& stash);

public:
    int getRandomLeaf();
    Client(int num_blocks);
    block access(int op, int id, const string& data = "");
    BucketHeap* getTree();
    vector<block> range_query(int start, int end);
};

#endif // CLIENT_H
