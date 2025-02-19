#include "../include/oram.h"

#include <cmath>
#include <iostream>

using namespace std;


//init with random blocks.
ORAM::ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey) {
    this->bucketCapacity = bucketCapacity;
    this->encryptionKey = encryptionKey;
    heap.reserve(numBuckets);
    for (int i = 0; i < numBuckets; i++) {
        heap.push_back(Bucket(bucketCapacity));
    }
}

// calculate the bit reverse of the number
int ORAM::bitReverse(int x, int bits) {
    int y = 0;
    for (int i = 0; i < bits; i++) {
        y = (y << 1) | (x & 1);
        x >>= 1;
    }
    return y;
}

// convert a physical (bit-reversed-lexographic) index to its normal index
int ORAM::toNormalIndex(int physicalIndex) {
    // get level of physical index
    int level = 0;
    int temp = physicalIndex + 1;
    while (temp >>= 1){
        level++;
    }
    int levelStart = (1 << level) - 1;

    // get position in the level
    int pos_br = physicalIndex - levelStart;
    // get bitreverse version - returns normal)
    int pos_normal = bitReverse(pos_br, level);
    return levelStart + pos_normal;
}

// convert a normal index to its physical (bit-reversed-lexographic index) - same thing as tonormalindex but its
// nice to have them seperated for my clarity
int ORAM::toPhysicalIndex(int normalIndex) {
    int level = 0;
    int temp = normalIndex + 1;
    while (temp >>= 1){
        level++;
    }
    int levelStart = (1 << level) - 1;
    int pos_normal = normalIndex - levelStart;
    int pos_br = bitReverse(pos_normal, level);
    return levelStart + pos_br;
}

int ORAM::parent(int i) {
    if (i == 0){
        return -1;
    }
    // convert physical index to normal index
    int normal_index = toNormalIndex(i);
    // compute parent's normal index
    int normal_parent = (normal_index - 1) / 2;
    // convert back to physical index
    return toPhysicalIndex(normal_parent);
}

int ORAM::leftChild(int i) {
    // convert physical index to normal index
    int normal_index = toNormalIndex(i);
    // compute left child's normal index
    int normal_left = 2 * normal_index + 1;
    // convert back to physical index
    return toPhysicalIndex(normal_left);
}

int ORAM::rightChild(int i) {
    // convert physical index to normal index
    int normal_index = toNormalIndex(i);
    // compute right child's normal index
    int normal_right = 2 * normal_index + 2;
    // convert back to physical index
    return toPhysicalIndex(normal_right);
}

// update a bucket based on the NORMAL INDEX
void ORAM::updateBucket(int normalIndex, const Bucket &newBucket) {
    int physicalIndex = toPhysicalIndex(normalIndex);
    if (physicalIndex < 0 || physicalIndex >= heap.size()) {
        throw std::out_of_range("Invalid bucket index");
    }
    heap[physicalIndex] = newBucket;
}

// print the logical ORAM, this is the normal bst layout.
void ORAM::print_logical_oram() {
    for (int normalIndex = 0; normalIndex < heap.size(); normalIndex++) {
        int physicalIndex = toPhysicalIndex(normalIndex);
        cout << "Logical Bucket " << normalIndex 
             << " (Physical index: " << physicalIndex << ")" << endl;
        heap[physicalIndex].print_bucket();
    }
}

// print the physical ORAM, this is the memory layout of the oram, it just prints through the heap
void ORAM::print_physical_oram() {
    for (int i = 0; i < heap.size(); i++) {
        cout << "Bucket " << i << endl;
        heap[i].print_bucket();
    }
}