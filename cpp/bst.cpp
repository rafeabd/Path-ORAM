#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

#define DEFAULT_CAPACITY 1000 // default capcity - idk if we need this its a remnant from 101

BucketHeap::BucketHeap(int numBuckets, int bucketCapacity) {
    for (int i = 0; i < numBuckets; i++) {
        heap.push_back(Bucket(bucketCapacity));
    }
}

int BucketHeap::parent(int i) { 
    return (i - 1) / 2; 
}

int BucketHeap::leftChild(int i) { 
    return 2 * i + 1; 
}

int BucketHeap::rightChild(int i) { 
    return 2 * i + 2; 
}

void BucketHeap::addBucket(const Bucket& bucket) {
    heap.push_back(bucket);
}

Bucket BucketHeap::removeBucket() {
    if (heap.empty())
        return Bucket(0);
        
    Bucket last = heap.back();
    heap.pop_back();
    return last;
}

Bucket& BucketHeap::getBucket(int index) {
    if (index >= 0 && index < heap.size())
        return heap[index];
    throw std::out_of_range("Index out of range");
}

bool BucketHeap::addBlockToBucket(int bucketIndex, const block& b) {
    if (bucketIndex >= 0 && bucketIndex < heap.size()) {
        return heap[bucketIndex].addBlock(b);
    }
    return false;
}


void BucketHeap::printHeap() {
    for (int i = 0; i < heap.size(); i++) {
        std::cout << "Bucket " << i << ":\n";
        std::cout << "  Parent: " << (i > 0 ? parent(i) : -1) << "\n";
        std::cout << "  Left Child: " << (leftChild(i) < heap.size() ? leftChild(i) : -1) << "\n";
        std::cout << "  Right Child: " << (rightChild(i) < heap.size() ? rightChild(i) : -1) << "\n";
        heap[i].print_bucket();
        std::cout << "\n";
    }
}

size_t BucketHeap::size() const {
    return heap.size();
}

bool BucketHeap::empty() const {
    return heap.empty();
}

