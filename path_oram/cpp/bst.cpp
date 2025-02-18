/**
 * @file bst.cpp
 * @brief Implements binary tree functionality for Path ORAM
 * 
 * This file contains the implementation of the BucketHeap class,
 * which provides the tree structure necessary for Path ORAM operations.
 * 
 * For rORAM extension, key areas to enhance include:
 * 1. Background eviction processes
 * 2. Level-aware bucket management
 * 3. Access pattern optimization
 * 4. Hierarchical storage support
 */

#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include "../include/encryption.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>
using namespace std;

/**
 * @brief Constructs a new BucketHeap
 * 
 * Initializes the tree structure with encrypted dummy blocks.
 * For rORAM, consider initializing:
 * - Level-specific bucket sizes
 * - Background processes
 * - Access pattern tracking
 */
BucketHeap::BucketHeap(int numBuckets, int bucketCapacity, const vector<unsigned char>& encKey)
    : bucketCapacity(bucketCapacity), encryptionKey(encKey)
{
    for (int i = 0; i < numBuckets; i++) {
        Bucket bucket(bucketCapacity);
        // this is dumb but need to clear buckets after they have been initialized
        bucket.clear();
        //add encrypted dummy blocks to oram buckets
        for (int j = 0; j < bucketCapacity; j++) {
            block dummyBlock(-1, -1, "dummy", true);
            dummyBlock = encryptBlock(dummyBlock, encryptionKey);
            bucket.startaddblock(dummyBlock);
        }
        heap.push_back(bucket);
    }
}

/**
 * @brief Gets parent node index
 * For rORAM: Consider level-specific parent relationships
 */
int BucketHeap::parent(int i) { 
    return (i - 1) / 2;
}

/**
 * @brief Gets left child index
 */
int BucketHeap::leftChild(int i) { 
    return 2 * i + 1;
}

/**
 * @brief Gets right child index
 */
int BucketHeap::rightChild(int i) { 
    return 2 * i + 2;
}

/**
 * @brief Adds new bucket to heap
 * For rORAM: Add level-aware bucket initialization
 */
void BucketHeap::addBucket(const Bucket& bucket) {
    heap.push_back(bucket);
}

/**
 * @brief Removes and returns last bucket
 */
Bucket BucketHeap::removeBucket() {
    Bucket last = heap.back();
    heap.pop_back();
    return last;
}

/**
 * @brief Gets bucket at specific index
 */
Bucket& BucketHeap::getBucket(int index) {
    if (index >= 0 && index < heap.size()) {
        return heap[index];
    }
    throw out_of_range("Index out of range");
}

/**
 * @brief Updates bucket at specific index
 * For rORAM: Add access pattern tracking
 */
void BucketHeap::updateBucket(int index, const Bucket& bucket) {
    if (index >= 0 && index < heap.size())
        heap[index] = bucket;
    else
        throw out_of_range("Index out of range");
}

/**
 * @brief Adds block to specific bucket
 */
bool BucketHeap::addBlockToBucket(int bucketIndex, const block& b) {
    if (bucketIndex >= 0 && bucketIndex < heap.size()) {
        return heap[bucketIndex].addBlock(b);
    }
    return false;
}

/**
 * @brief Prints heap structure
 * For rORAM: Add level and access pattern information
 */
void BucketHeap::printHeap() {
    for (int i = 0; i < heap.size(); i++) {
        cout << "Bucket " << i << ":\n";
        heap[i].print_bucket();
        cout << "\n";
    }
}

/**
 * @brief Gets heap size
 */
size_t BucketHeap::size() const {
    return heap.size();
}

/**
 * @brief Checks if heap is empty
 */
bool BucketHeap::empty() const {
    return heap.empty();
}

/**
 * @brief Gets blocks along path from leaf
 * For rORAM: Add level-aware path retrieval and optimization
 */
vector<block> BucketHeap::getPathFromLeaf(int leafIndex) {
    vector<block> path;
    int current = leafIndex;
    while (true) {
        vector<block> blocks = heap[current].getBlocks();
        path.insert(path.end(), blocks.begin(), blocks.end());
        if (current == 0) break;  
        current = parent(current);
    }
    reverse(path.begin(), path.end());
    return path;
}

/**
 * @brief Gets buckets along path from leaf
 * For rORAM: Implement level-specific bucket retrieval
 */
vector<Bucket> BucketHeap::getPathBuckets(int leafIndex) {
    vector<Bucket> path;
    
    int current = leafIndex;
    while (true) {
        path.push_back(heap[current]);
        if (current == 0) break;  
        current = parent(current);
    }
    reverse(path.begin(), path.end());
    return path;
}

/**
 * @brief Gets indices of path from leaf to root
 */
vector<int> BucketHeap::getPathIndices(int leaf) {
    vector<int> path;
    
    int current = leaf;
    while (current >= 0) {
        path.push_back(current);

        if (current == 0) break;
        current = parent(current);
    }
    
    return path;
}

/**
 * @brief Clears bucket at given index
 * For rORAM: Add background eviction support
 */
void BucketHeap::clear_bucket(int index) {
    Bucket newBucket(bucketCapacity);
    for (int j = 0; j < bucketCapacity; j++) {
        block dummyBlock(-1, -1, "dummy", true);
        dummyBlock = encryptBlock(dummyBlock, encryptionKey);
        newBucket.addBlock(dummyBlock);
    }
    heap[index] = newBucket;
}

/* TODO for rORAM - Additional methods to consider:
 * 
 * void performBackgroundEviction()
 * - Implements continuous background eviction process
 * - Maintains balanced block distribution
 * - Optimizes based on access patterns
 * 
 * void adjustBucketCapacity(int level)
 * - Modifies bucket capacity based on tree level
 * - Optimizes storage utilization
 * - Handles capacity transitions
 * 
 * void trackAccessPattern(int bucketIndex)
 * - Records bucket access frequency
 * - Updates hot/cold classification
 * - Guides optimization decisions
 * 
 * void optimizePath(const vector<int>& path)
 * - Reorganizes blocks along path
 * - Implements level-aware placement
 * - Improves access efficiency
 */
