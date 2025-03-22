#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include "../include/encryption.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>
using namespace std;

BucketHeap::BucketHeap(int numBuckets, int bucketCapacity, const vector<unsigned char>& encKey)
    : bucketCapacity(bucketCapacity), encryptionKey(encKey)
{
    for (int i = 0; i < numBuckets; i++) {
        Bucket bucket(bucketCapacity);
        // this is dumb but need to clear buckets after they have been initialized
        bucket.clear();
        //add encrypted dummy blocks to oram buckets
        for (int j = 0; j < bucketCapacity; j++) {
            block dummyBlock(-1, -1, "dummy", true);
            dummyBlock = encryptBlock(dummyBlock, encryptionKey);
            bucket.startaddblock(dummyBlock);
        }
        heap.push_back(bucket);
    }
}

// Returns the index of the parent node of the given index in a binary heap.
int BucketHeap::parent(int i) { 
    return (i - 1) / 2;  // Integer division finds the parent index.
}

// Returns the index of the left child of the given index in a binary heap.
int BucketHeap::leftChild(int i) { 
    return 2 * i + 1;  // Left child index formula in a heap.
}

// Returns the index of the right child of the given index in a binary heap.
int BucketHeap::rightChild(int i) { 
    return 2 * i + 2;  // Right child index formula in a heap.
}

// Adds a new Bucket to the heap.
void BucketHeap::addBucket(const Bucket& bucket) {
    heap.push_back(bucket);  // Push the new bucket onto the heap vector.
}

// Removes and returns the last Bucket from the heap.
Bucket BucketHeap::removeBucket() {
    Bucket last = heap.back();  // Store the last bucket in a temporary variable.
    heap.pop_back();            // Remove the last bucket from the heap.
    return last;                // Return the removed bucket.

}

// Retrieves a reference to the Bucket at a specific index in the heap.
Bucket& BucketHeap::getBucket(int index) {
    if (index >= 0 && index < heap.size()) {
        return heap[index];  // Return the bucket at the given index.
    }
    throw out_of_range("Index out of range");  // Throw an exception if index is invalid.
}

void BucketHeap::updateBucket(int index, const Bucket& bucket) {
    if (index >= 0 && index < heap.size())
        heap[index] = bucket;
    else
        throw out_of_range("Index out of range");
}

// Adds a Block to a specific Bucket in the heap.
bool BucketHeap::addBlockToBucket(int bucketIndex, const block& b) {
    if (bucketIndex >= 0 && bucketIndex < heap.size()) {
        return heap[bucketIndex].addBlock(b);  // Call addBlock() on the targeted bucket.
    }
    return false;  // Return false if the index is out of bounds.
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

// Returns the number of Buckets in the heap.
size_t BucketHeap::size() const {
    return heap.size();  // Return the size of the heap vector.
}

// Checks if the heap is empty.
bool BucketHeap::empty() const {
    return heap.empty();  // Return true if the heap is empty, otherwise false.
}

// Returns a vector containing the path from a leaf bucket to the root.
vector<block> BucketHeap::getPathFromLeaf(int leafIndex) {
    vector<block> path;  // Vector to store blocks along the path.
    int current = leafIndex;  // Start at the given leaf index.
    while (true) {
        vector<block> blocks = heap[current].getBlocks();
        path.insert(path.end(), blocks.begin(), blocks.end());
        if (current == 0) break;  
        current = parent(current);
    }
    reverse(path.begin(), path.end());  // Reverse the path so it's ordered from root to leaf.
    return path;
}

vector<Bucket> BucketHeap::getPathBuckets(int leafIndex) {
    vector<Bucket> path;
    
    int current = leafIndex;
    while (true) {
        path.push_back(heap[current]);
        if (current == 0) break;  
        current = parent(current);
    }
    reverse(path.begin(), path.end());
    return path;
}

// Returns a vector of indices representing the path from a leaf to the root.
vector<int> BucketHeap::getPathIndices(int leaf){
    vector<int> path;
    
    int current = leaf;
    // Build path from leaf to root
    while (current >= 0) {
        path.push_back(current);  // Add the current index to the path

        if (current == 0) break;  // Stop when reaching the root
        current = parent(current);  // Move up to the parent
    }
    
    return path;
}

void BucketHeap::clear_bucket(int index) {
    // Reinitialize bucket with encrypted dummy blocks
    Bucket newBucket(bucketCapacity);
    for (int j = 0; j < bucketCapacity; j++) {
        block dummyBlock(-1, -1, "dummy", true);
        dummyBlock = encryptBlock(dummyBlock, encryptionKey);
        newBucket.addBlock(dummyBlock);
    }
    heap[index] = newBucket;
}