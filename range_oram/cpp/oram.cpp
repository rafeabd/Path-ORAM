#include "../include/oram.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <set>

using namespace std;

// Constructor: Initialize ORAM tree with numBuckets buckets,
// each with capacity bucketCapacity. The encryptionKey and range_length
// parameters are passed in (encryptionKey is not used in this snippet).
ORAM::ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length) {
    this->bucketCapacity = bucketCapacity;
    this->num_buckets = numBuckets;
    this->range_length = range_length;
    this->global_counter = 0;
    heap.reserve(numBuckets);
    for (int i = 0; i < numBuckets; i++) {
        heap.push_back(Bucket(bucketCapacity));
    }
}

// Compute the bit-reverse of x using the specified number of bits.
int ORAM::bitReverse(int x, int bits) {
    int y = 0;
    for (int i = 0; i < bits; i++) {
        y = (y << 1) | (x & 1);
        x >>= 1;
    }
    return y;
}

// Convert a physical (bit-reversed) index to its normal (logical) index.
int ORAM::toNormalIndex(int physicalIndex) {
    int level = 0;
    int temp = physicalIndex + 1;
    while (temp >>= 1) {
        level++;
    }
    int levelStart = (1 << level) - 1;
    int pos_br = physicalIndex - levelStart;
    int pos_normal = bitReverse(pos_br, level);
    return levelStart + pos_normal;
}

// Convert a normal (logical) index to its physical (bit-reversed) index.
int ORAM::toPhysicalIndex(int normalIndex) {
    int level = 0;
    int temp = normalIndex + 1;
    while (temp >>= 1) {
        level++;
    }
    int levelStart = (1 << level) - 1;
    int pos_normal = normalIndex - levelStart;
    int pos_br = bitReverse(pos_normal, level);
    return levelStart + pos_br;
}

// Convert a leaf index (0-based) to the corresponding physical index at the leaf level.
int ORAM::leafToPhysicalIndex(int leaf) {
    int height = ceil(log2(num_buckets + 1));
    int leafLevel = height - 1;
    int firstLeafIndex = (1 << leafLevel) - 1;
    int normalIndex = firstLeafIndex + leaf;
    return toPhysicalIndex(normalIndex);
}

// Return the physical index of the parent of node i.
int ORAM::parent(int i) {
    if (i == 0) {
        return -1;
    }
    int normal_index = toNormalIndex(i);
    int normal_parent = (normal_index - 1) / 2;
    return toPhysicalIndex(normal_parent);
}

// Return the physical index of the left child of node i.
int ORAM::leftChild(int i) {
    int normal_index = toNormalIndex(i);
    int normal_left = 2 * normal_index + 1;
    return toPhysicalIndex(normal_left);
}

// Return the physical index of the right child of node i.
int ORAM::rightChild(int i) {
    int normal_index = toNormalIndex(i);
    int normal_right = 2 * normal_index + 2;
    return toPhysicalIndex(normal_right);
}

// Read the bucket at the given logical index (using normal indexing).
Bucket ORAM::read_bucket(int logical_index) {
    return heap[toPhysicalIndex(logical_index)];
}

// Read a range of buckets at a given level starting from a given physical node (wrap-around).
vector<Bucket> ORAM::read_level_range(int level, int physical_node, int range_length) {
    vector<Bucket> result;
    int levelStart = (1 << level) - 1;
    int levelCount = min((1 << level), num_buckets - levelStart);
    int offset = physical_node - levelStart;
    for (int i = 0; i < range_length; i++) {
        int pos = (offset + i) % levelCount;
        result.push_back(heap[levelStart + pos]);
    }
    return result;
}

// Read all buckets along the path from a given leaf (returns a vector of vectors,
// each inner vector holding the bucket at that level).
vector<vector<Bucket>> ORAM::read_range(int leaf) {
    vector<vector<Bucket>> result;
    vector<int> path_indices = getpathindicies_ltor(leaf);
    for (size_t i = 0; i < path_indices.size(); i++) {
        vector<Bucket> level_buckets;
        level_buckets.push_back(heap[path_indices[i]]);
        result.push_back(level_buckets);
    }
    return result;
}

// Compute path indices from leaf to root (returns physical indices).
vector<int> ORAM::getpathindicies_ltor(int leaf) {
    vector<int> path_indices;
    int physical_leaf = leafToPhysicalIndex(leaf);
    int current = physical_leaf;
    while (current >= 0) {
        path_indices.push_back(current);
        current = parent(current);
    }
    return path_indices;
}

// Compute path indices from root to leaf (returns physical indices).
vector<int> ORAM::getpathindicies_rtol(int leaf) {
    vector<int> path_indices = getpathindicies_ltor(leaf);
    reverse(path_indices.begin(), path_indices.end());
    return path_indices;
}

// Retrieve the bucket at a given level and index within that level (normal indexing for that level).
Bucket ORAM::get_bucket_at_level(int level, int index_in_level) {
    int levelStart = (1 << level) - 1;
    int levelCount = min((1 << level), num_buckets - levelStart);
    if (index_in_level < 0 || index_in_level >= levelCount) {
        throw std::out_of_range("Bucket index out of range for the specified level");
    }
    int normalIndex = levelStart + index_in_level;
    int physicalIndex = toPhysicalIndex(normalIndex);
    return heap[physicalIndex];
}

// Update the bucket at the given logical (normal) index with the new bucket.
void ORAM::updateBucket(int logicalIndex, const Bucket &newBucket) {
    int physicalIndex = toPhysicalIndex(logicalIndex);
    if (physicalIndex < 0 || physicalIndex >= heap.size()) {
        throw std::out_of_range("Invalid bucket index");
    }
    heap[physicalIndex] = newBucket;
}


vector<Bucket> ORAM::readBucketsAtLevel(int level, int start_index, int count) {
    int levelStart = (1 << level) - 1;
    int levelCount = min((1 << level), num_buckets - levelStart);
    set<int> indices;
    for (int t = start_index; t < start_index + count; ++t) {
        indices.insert(t % levelCount);
    }
    vector<Bucket> buckets;
    for (int idx : indices) {
        // Convert level index to physical and fetch bucket
        int normalIndex = levelStart + idx;
        if (normalIndex >= num_buckets) continue;  // skip if beyond tree size (safety check)
        int physicalIndex = toPhysicalIndex(normalIndex);
        buckets.push_back(heap[physicalIndex]);
    }
    return buckets;
}

/**
 * Read all buckets at the given level for the range [start_index, start_index + count),
 * and clear those buckets in the ORAM tree (replace with dummy buckets). 
 * Returns the vector of buckets read.
 */
vector<Bucket> ORAM::readBucketsAndClear(int level, int start_index, int count) {
    int levelStart = (1 << level) - 1;
    int levelCount = min((1 << level), num_buckets - levelStart);
    set<int> indices;
    for (int t = start_index; t < start_index + count; ++t) {
        indices.insert(t % levelCount);
    }
    vector<Bucket> buckets;
    for (int idx : indices) {
        int normalIndex = levelStart + idx;
        if (normalIndex >= num_buckets) continue;
        int physicalIndex = toPhysicalIndex(normalIndex);
        buckets.push_back(heap[physicalIndex]);
        // Replace the bucket with an empty bucket of equal capacity
        heap[physicalIndex] = Bucket(bucketCapacity);
    }
    return buckets;
}

/**
 * Update the bucket at the given level and index within that level with a new bucket.
 * (Level is 0-indexed, index_in_level is the bucket's position at that level in normal ordering.)
 */
void ORAM::updateBucketAtLevel(int level, int index_in_level, const Bucket &newBucket) {
    int levelStart = (1 << level) - 1;
    int levelCount = min((1 << level), num_buckets - levelStart);
    if (index_in_level < 0 || index_in_level >= levelCount) {
        throw std::out_of_range("Bucket index out of range for the specified level");
    }
    int normalIndex = levelStart + index_in_level;
    updateBucket(normalIndex, newBucket);
}
