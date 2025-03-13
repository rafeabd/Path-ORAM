#ifndef SERVER_H
#define SERVER_H

#include "bucket.h"
#include "oram.h"
#include <vector>

using namespace std;

class Server {
private:
    BucketHeap oram;    
    int L;
    int Z;
public:
    Server(int num_blocks, int bucket_size, BucketHeap initialized_tree);
    vector<Bucket> give_path(int leaf);
    void write_bucket( Bucket& path, int bucket_index);
    void printHeap();
};

#endif 