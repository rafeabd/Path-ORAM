#ifndef ORAM_H
#define ORAM_H

#include <vector>
#include "bucket.h"

using namespace std;

class ORAM {
private:

    
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);
public:
    vector<Bucket> heap;
    int bucketCapacity;
    
    int global_counter;
    int num_buckets;
    int range_length;
    ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length);
    int bitReverse(int x, int bits);
    void print_physical_oram(bool split_levels = false);
    int toNormalIndex(int physicalIndex);
    int toPhysicalIndex(int normalIndex);
    void updateBucket(int normalIndex, const Bucket &newBucket);
    void print_logical_oram();
    vector<int> getpathindicies_ltor(int leaf);
    vector<int> getpathindicies_rtol(int leaf);
    Bucket read_bucket(int logical_index);
    vector<Bucket> read_level_range(int level, int physical_node, int range_length);
    vector<vector<Bucket> > read_range(int leaf);
    bool stashContains(int address);
    void removeFromStash(vector<block>& S);
    vector<int> get_path_labels_mod(int leaf);
    Bucket get_bucket_at_level(int level, int index_in_level);
    int leafToPhysicalIndex(int leaf);
    void updateBucketAtLevel(int level, int index_in_level, const Bucket &newBucket);
    vector<Bucket> readBucketsAndClear(int level, int start_index, int count);
    vector<Bucket> readBucketsAtLevel(int level, int start_index, int count);
};

#endif
