#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/bst.h"
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

vector<block> Server::give_path(int leaf) {
    return oram.getPathFromLeaf(leaf);
}

bool Server::write_block_to_path(const block& b, int leaf) {
    vector<int> path = oram.getPathIndices(leaf);
    
    // tries to place block in the path if there is space. if not, returns false.
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        int bucket_index = *it;
        if (oram.addBlockToBucket(bucket_index, b)) {
            return true;
        }
    }
    return false;
}
