#ifndef BUCKET_HEAP_H
#define BUCKET_HEAP_H

#include <vector>
#include "bucket.h"
#include "block.h"

using namespace std;

class BucketHeap {
private:
    vector<Bucket> heap;
    
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);

public:
    BucketHeap(int numBuckets, int bucketCapacity);
    void addBucket(const Bucket& bucket);
    Bucket removeBucket();
    Bucket& getBucket(int index);
    bool addBlockToBucket(int bucketIndex, const block& b);
    void printHeap();
    size_t size() const;
    bool empty() const;
};

#endif