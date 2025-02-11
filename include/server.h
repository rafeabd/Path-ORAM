#ifndef SERVER_H
#define SERVER_H

#include "bucket.h"
#include "bst.h"
#include <vector>

using namespace std;

class Server {
private:
    BucketHeap oram;    
    int L;              
    int Z;              
public:
    Server(int num_blocks, int bucket_size, BucketHeap initialized_tree);
    vector<block> give_path(int leaf);
    
    bool write_block_to_path(const block& b, int leaf);
};

#endif 
