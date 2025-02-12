#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include <iostream>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

// Constructor for the BucketHeap class.
// Initializes the heap with a given number of Buckets, each with a specified capacity.
BucketHeap::BucketHeap(int numBuckets, int bucketCapacity) {
    for (int i = 0; i < numBuckets; i++) {
        heap.push_back(Bucket(bucketCapacity));  // Create and add new buckets to the heap.
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
        vector<block> blocks = heap[current].getBlocks();  // Get blocks from the current bucket.
        path.insert(path.end(), blocks.begin(), blocks.end());  // Append blocks to the path.
        if (current == 0) break;  // Stop when reaching the root.
        current = parent(current);  // Move up to the parent bucket.
    }
    reverse(path.begin(), path.end());  // Reverse the path so it's ordered from root to leaf.
    return path;
}

// Returns a vector of indices representing the path from a leaf to the root.
vector<int> BucketHeap::getPathIndices(int leaf){
    vector<int> path;  // Vector to store the indices of the path.
    int current = leaf;  // Start at the given leaf index.
    
    // Traverse from leaf to root, storing each index along the way.
    while (current >= 0) {
        path.push_back(current);  // Add the current index to the path.

        if (current == 0) break;  // Stop when reaching the root.
        current = parent(current);  // Move up to the parent.
    }
    
    return path;  // Return the list of indices representing the path.
}
