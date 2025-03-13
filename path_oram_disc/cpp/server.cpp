#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/oram.h"
#include "../include/block.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

Server::Server(int num_blocks, int bucket_size, BucketHeap initialized_tree)
    : Z(bucket_size),
      L(ceil(log2(num_blocks))),
      oram(move(initialized_tree)) {}

vector<Bucket> Server::give_path(int leaf) {
    int bucket_index = leaf + ((1 << L) - 1);
    
    // Get the indices first
    vector<int> pathIndices = oram.getPathIndices(bucket_index);
    vector<Bucket> path = oram.getPathBuckets(bucket_index);
    //for (Bucket bucket: path){
    //    bucket.print_bucket();
    //}

    for (int id : pathIndices) {
        oram.clear_bucket(id);  
    }
    return path;
}

void Server::write_bucket(const Bucket& path, int bucket_index) {
    oram.updateBucket(bucket_index, path);
}
