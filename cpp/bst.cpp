#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>
using namespace std;

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
    Bucket last = heap.back();
    heap.pop_back();
    return last;
}

Bucket& BucketHeap::getBucket(int index) {
    if (index >= 0 && index < heap.size())
        return heap[index];
    throw out_of_range("Index out of range");
}

void BucketHeap::updateBucket(int index, const Bucket& bucket) {
    if (index >= 0 && index < heap.size())
        heap[index] = bucket;
    else
        throw out_of_range("Index out of range");
}

bool BucketHeap::addBlockToBucket(int bucketIndex, const block& b) {
    if (bucketIndex >= 0 && bucketIndex < heap.size()) {
        return heap[bucketIndex].addBlock(b);
    }
    return false;
}

void BucketHeap::printHeap() {
    /*
    for (int i = 0; i < heap.size(); i++) {
        cout << "Bucket " << i << ":\n";
        cout << "  Parent: " << (i > 0 ? parent(i) : -1) << "\n";
        cout << "  Left Child: " << (leftChild(i) < heap.size() ? leftChild(i) : -1) << "\n";
        cout << "  Right Child: " << (rightChild(i) < heap.size() ? rightChild(i) : -1) << "\n";
        heap[i].print_bucket();
        cout << "\n";
    }
    */

    for (int i = 0; i < heap.size(); i++) {
        cout << "Bucket " << i << ":\n";
        heap[i].print_bucket();
        cout << "\n";
    }
}

size_t BucketHeap::size() const {
    return heap.size();
}

bool BucketHeap::empty() const {
    return heap.empty();
}

vector<block> BucketHeap::getPathFromLeaf(int leafIndex) {
    vector<block> path;
    int current = leafIndex;
    while (true) {
        vector<block> blocks = heap[current].getBlocks();
        path.insert(path.end(), blocks.begin(), blocks.end());
        if (current == 0) break;  // Break once we hit the root.
        current = parent(current);
    }
    reverse(path.begin(), path.end());
    return path;
}

vector<Bucket> BucketHeap::getPathBuckets(int leafIndex) {
    vector<Bucket> path;
    int current = leafIndex;
    while (true) {
        path.push_back(heap[current]);
        if (current == 0) break;  // Break once we hit the root.
        current = parent(current);
    }
    reverse(path.begin(), path.end());
    return path;
}

vector<int> BucketHeap::getPathIndices(int leaf){
    vector<int> path;
    int current = leaf;
    
    // Build path from leaf to root
    while (current >= 0) {
        path.push_back(current);
        if (current == 0) break;
        current = parent(current);
    }
    
    return path;
}

void BucketHeap::clear_bucket(int index) {
    heap[index] = Bucket();
}
