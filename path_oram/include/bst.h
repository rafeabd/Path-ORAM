/**
 * @file bst.h
 * @brief Defines the binary tree structure for Path ORAM storage.
 * 
 * The BucketHeap class implements a binary tree where each node is a bucket.
 * This structure is fundamental to Path ORAM operations, allowing efficient
 * path retrieval and updates.
 * 
 * For rORAM extension, this structure would need to support:
 * 1. Hierarchical levels of storage
 * 2. Different bucket sizes per level
 * 3. Background eviction processes
 * 4. Access pattern-based optimization
 */

#ifndef BUCKET_HEAP_H
#define BUCKET_HEAP_H

#include <vector>
#include "bucket.h"
#include "block.h"

using namespace std;

/**
 * @class BucketHeap
 * @brief Binary tree implementation for Path ORAM
 * 
 * Current implementation uses a vector-based heap structure.
 * For rORAM, consider extending with:
 * - Level-specific bucket management
 * - Dynamic bucket sizing
 * - Background operations support
 * - Access pattern tracking
 */
class BucketHeap {
private:
    vector<Bucket> heap;           // Binary tree structure
    int bucketCapacity;           // Fixed capacity per bucket
    vector<unsigned char> encryptionKey;  // Encryption key for secure operations
    
    /**
     * @brief Gets parent node index
     * @param i Current node index
     * @return Parent node index
     * 
     * For rORAM: Consider adding level-aware parent calculation
     */
    int parent(int i);

    /**
     * @brief Gets left child index
     * @param i Current node index
     * @return Left child index
     */
    int leftChild(int i);

    /**
     * @brief Gets right child index
     * @param i Current node index
     * @return Right child index
     */
    int rightChild(int i);

public:
    /**
     * @brief Constructs a new BucketHeap
     * @param numBuckets Number of buckets in the tree
     * @param bucketCapacity Capacity of each bucket
     * @param encKey Encryption key for secure operations
     * 
     * For rORAM extension:
     * - Add support for variable bucket capacities
     * - Initialize background processes
     * - Setup access pattern tracking
     */
    BucketHeap(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey);

    /**
     * @brief Adds a new bucket to the heap
     * @param bucket Bucket to add
     */
    void addBucket(const Bucket& bucket);

    /**
     * @brief Removes and returns the last bucket
     * @return Removed bucket
     */
    Bucket removeBucket();

    /**
     * @brief Gets bucket at specific index
     * @param index Bucket index
     * @return Reference to bucket
     */
    Bucket& getBucket(int index);

    /**
     * @brief Updates bucket at specific index
     * @param index Bucket index
     * @param bucket New bucket value
     */
    void updateBucket(int index, const Bucket& bucket);

    /**
     * @brief Adds block to specific bucket
     * @param bucketIndex Target bucket index
     * @param b Block to add
     * @return true if addition successful
     */
    bool addBlockToBucket(int bucketIndex, const block& b);

    /**
     * @brief Prints heap structure for debugging
     */
    void printHeap();

    /**
     * @brief Gets number of buckets in heap
     * @return Heap size
     */
    size_t size() const;

    /**
     * @brief Checks if heap is empty
     * @return true if empty
     */
    bool empty() const;

    /**
     * @brief Gets blocks along path from leaf
     * @param leafIndex Leaf node index
     * @return Vector of blocks in path
     * 
     * For rORAM:
     * - Add level-aware path retrieval
     * - Implement access pattern tracking
     * - Support background operations
     */
    vector<block> getPathFromLeaf(int leafIndex);

    /**
     * @brief Gets indices of path from leaf to root
     * @param leaf Leaf node index
     * @return Vector of indices in path
     */
    vector<int> getPathIndices(int leaf);

    /**
     * @brief Gets buckets along path from leaf
     * @param leafIndex Leaf node index
     * @return Vector of buckets in path
     */
    vector<Bucket> getPathBuckets(int leafIndex);

    /**
     * @brief Clears bucket at given index
     * @param index Bucket index to clear
     * 
     * For rORAM:
     * - Add background eviction support
     * - Implement level-aware clearing
     */
    void clear_bucket(int index);
};

#endif
