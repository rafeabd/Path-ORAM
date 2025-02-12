#include "../include/server.h"  // Include the Server class (manages ORAM storage).
#include "../include/bucket.h"  // Include the Bucket class (stores blocks in ORAM tree nodes).
#include "../include/bst.h"     // Include BST (possibly used for additional storage structures).
#include "../include/block.h"   // Include Block (represents data stored in ORAM).
#include <vector>               // Include vector for dynamic arrays.
#include <iostream>             // Include for console output.
#include <cmath>                // Include for mathematical operations like log2().
#include <algorithm>            // Include for algorithms like reverse iteration.

using namespace std;

/**
 * @brief Server constructor initializes ORAM storage.
 * 
 * The server manages a **binary ORAM tree** using `BucketHeap`, 
 * where each node (bucket) can store `Z` blocks.
 * 
 * @param num_blocks The total number of blocks stored in ORAM.
 * @param bucket_size The maximum number of blocks a bucket can hold.
 * @param initialized_tree The pre-initialized ORAM binary tree (BucketHeap).
 */
Server::Server(int num_blocks, int bucket_size, BucketHeap initialized_tree)
    : Z(bucket_size),                  // Store the bucket capacity.
      L(ceil(log2(num_blocks))),        // Compute the tree height L based on the number of blocks.
      oram(move(initialized_tree)) {}   // Store the ORAM tree using move semantics.

/**
 * @brief Retrieves all blocks along the path from a given leaf to the root.
 * 
 * In Path ORAM, every access (read or write) must involve retrieving 
 * **an entire path** from the tree. This ensures that access patterns 
 * remain oblivious and attackers cannot infer which block is being accessed.
 * 
 * @param leaf The index of the leaf node from which the path is retrieved.
 * @return A vector containing all blocks from the path.
 */
vector<block> Server::give_path(int leaf) {
    return oram.getPathFromLeaf(leaf);  // Retrieve the path from the ORAM structure.
}

/**
 * @brief Attempts to write a block back into the ORAM tree along its assigned path.
 * 
 * Path ORAM requires that after accessing a block, it must be **evicted** back 
 * into the ORAM tree along the same path, following **reverse eviction order** 
 * (from the root to the leaf).
 * 
 * @param b The block to be written into the ORAM.
 * @param leaf The leaf index where the block was previously assigned.
 * @return True if the block was successfully placed, false if no space was available.
 */
bool Server::write_block_to_path(const block& b, int leaf) {
    vector<int> path = oram.getPathIndices(leaf);  // Get the list of bucket indices from leaf to root.

    // Try to place the block into the first available bucket in the eviction path.
    for (auto it = path.rbegin(); it != path.rend(); ++it) {  // Reverse iterate from root to leaf.
        int bucket_index = *it;  // Get the current bucket index.

        // Attempt to insert the block into the bucket.
        if (oram.addBlockToBucket(bucket_index, b)) {
            return true;  // Successfully inserted the block, return true.
        }
    }
    return false;  // If all buckets were full, return false.
}
