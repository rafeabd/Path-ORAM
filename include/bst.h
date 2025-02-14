#ifndef BUCKET_HEAP_H
#define BUCKET_HEAP_H

#include <vector>
#include "bucket.h"
#include "block.h"

using namespace std;

class BucketHeap {
private:
    vector<Bucket> heap;
    int bucketCapacity;
    vector<unsigned char> encryptionKey;
    
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);
public:
    BucketHeap(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey);
    void addBucket(const Bucket& bucket);
    Bucket removeBucket();
    Bucket& getBucket(int index);
    void updateBucket(int index, const Bucket& bucket);
    bool addBlockToBucket(int bucketIndex, const block& b);
    void printHeap();
    size_t size() const;
    bool empty() const;
    vector<block> getPathFromLeaf(int leafIndex);
    vector<int> getPathIndices(int leaf);
    vector<Bucket> getPathBuckets(int leafIndex);
    void clear_bucket(int index);
};

#endif
