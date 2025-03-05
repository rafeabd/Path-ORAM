#ifndef ORAM_H
#define ORAM_H

#include <vector>
#include "bucket.h"
#include "block.h"

using namespace std;

class ORAM {
private:
    

public:
    vector<Bucket> heap;
    int bucketCapacity;
    int global_counter;
    int num_buckets;
    int range_length;
    
    ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length);

    int parent(int i);
    
    int bitReverse(int x, int bits);
    int toNormalIndex(int physicalIndex);
    int toPhysicalIndex(int normalIndex);

    int leafToPhysicalIndex(int leaf);
    Bucket read_bucket(int logical_index);
    vector<Bucket> readBucketsAndClear(int level, int start_index, int count);
    void updateBucket(int logicalIndex, const Bucket &newBucket);
    void updateBucketAtLevel(int level, int index_in_level, const Bucket &newBucket);
    vector<int> getpathindicies_ltor(int leaf);

    block writeBlockToPath(const block &b, int logicalLeaf);
    vector<Bucket> try_buckets_at_level(int level, int leaf, int range_power);
};

#endif
