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

class Client {
private:
    std::vector<unsigned char> key_for_id;
    std::vector<unsigned char> key_for_data;
    std::vector<block> stash;
    std::map<int, int> position_map;
    int L;
    std::shared_ptr<BucketHeap> tree;  // The oram

    int getRandomLeaf();
    bool isOnPath(int, int);
    std::vector<int> getPath(int leaf);
    std::vector<block> readPath(int leaf);
    void writePath(int leaf, std::vector<block>& stash);

    void evict(const std::vector<block>& );

public:
    Client(int num_blocks);
    block access(int op, int id, const std::string& data = "");
    BucketHeap* getTree();
};

#endif // CLIENT_H
