#ifndef ORAM_H
#define ORAM_H

#include <vector>
#include <fstream>
#include "bucket.h"

using namespace std;

class ORAM {
private:
    vector<unsigned char> encryptionKey;
    
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);
public:
    ~ORAM();
    fstream tree_file;
    string file_path;
    int bucketCapacity;
    
    int global_counter;
    int num_buckets;
    int range_length;
    ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length, string file);


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

    vector<Bucket> simple_buckets_at_level(int level, int leaf, int range_power);
    void simple_update_bucket(int level, int inx_in_level, Bucket updated_bucket);

    int simple_toPhysical(int index, int level);
    block writeBlockToPath(const block &b, int logicalLeaf, vector<unsigned char> key);
    vector<Bucket> try_buckets_at_level(int level, int leaf, int range_power);

    Bucket read_bucket_physical(int physicalIndex);
    Bucket read_bucket_physical_clear(int physicalIndex);

    void updateBucket_physical(int physicalIndex, const Bucket &newBucket);
    vector<Bucket> read_bucket_physical_consecutive(int physicalIndex, int range);

    void reopenFile();
    void flushCache();
    void updateBucketForInitialization(int logicalIndex, const Bucket &newBucket);
    void updateBucketsAtLevel(int level, const vector<pair<int, Bucket>>& indexBucketPairs);
    void writeContiguousLevel(int physicalStart, int count, const string &data);

};

#endif
