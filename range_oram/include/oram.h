#ifndef ORAM_H
#define ORAM_H

#include <vector>
#include "bucket.h"

using namespace std;

class ORAM {
private:
    vector<Bucket> heap;
    int bucketCapacity;
    vector<unsigned char> encryptionKey;
    
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);
public:
    ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey);
    int bitReverse(int x, int bits);
    void print_physical_oram();
    int toNormalIndex(int physicalIndex);
    int toPhysicalIndex(int normalIndex);
    void updateBucket(int normalIndex, const Bucket &newBucket);
    void print_logical_oram();

};

#endif
