#ifndef SERVER_H
#define SERVER_H

#include "bucket.h"
#include <vector>

using namespace std;

class Server {
private:
    vector<Bucket> tree;    // Binary tree stored as array
    const int L;                 // Tree height
    const int Z;                 // Bucket size

    vector<int> getPathToLeaf(int leaf) const;
    void initializeWithDummyBlocks();

public:
    Server(int num_blocks, int bucket_size = 4);
    vector<block> readPath(int leaf);
    void writePath(int leaf, const vector<block>& blocks);
};

#endif