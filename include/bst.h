#ifndef BUCKET_HEAP_H
#define BUCKET_HEAP_H

#include <vector>
#include "bucket.h"
#include "block.h"

class BucketHeap {
private:
    std::vector<Bucket> heap;
    
    // Helper functions for tree navigation
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);

public:
    // Constructor
    BucketHeap(int numBuckets, int bucketCapacity);
    
    // Core operations
    void addBucket(const Bucket& bucket);
    Bucket removeBucket();
    Bucket& getBucket(int index);
    bool addBlockToBucket(int bucketIndex, const block& b);
    
    // Utility functions
    void printHeap();
    size_t size() const;
    bool empty() const;
};

#endif // BUCKET_HEAP_H