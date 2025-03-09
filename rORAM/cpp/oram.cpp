#include "../include/oram.h"
#include "../include/encryption.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <set>

using namespace std;

ORAM::ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length) {
    this->encryption_key = encryptionKey;
    this->bucketCapacity = bucketCapacity;
    this->num_buckets = numBuckets;
    this->range_length = range_length;
    this->global_counter = 0;
    heap.reserve(numBuckets);
    for (int i = 0; i < numBuckets; i++) {
        heap.push_back(encrypt_bucket(Bucket(bucketCapacity), encryption_key));
    }
}

int ORAM::bitReverse(int x, int bits) {
    int y = 0;
    for (int i = 0; i < bits; i++) {
        y = (y << 1) | (x & 1);
        x >>= 1;
    }
    return y;
}

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

int ORAM::leafToPhysicalIndex(int leaf) {
    int height = ceil(log2(num_buckets + 1)) - 1;
    int leafLevel = height - 1;
    int firstLeafIndex = (1 << leafLevel) - 1;
    int normalIndex = firstLeafIndex + leaf;
    return toPhysicalIndex(normalIndex);
}

int ORAM::parent(int i) {
    if (i == 0) return -1;
    int normal_index = toNormalIndex(i);
    int normal_parent = (normal_index - 1) / 2;
    return toPhysicalIndex(normal_parent);
}

Bucket ORAM::read_bucket(int logical_index) {
    //cout << "logical index" << endl;
    return heap[toPhysicalIndex(logical_index)];
}

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
        heap[physicalIndex] = encrypt_bucket(Bucket(bucketCapacity),encryption_key);
    }
    return buckets;
}

void ORAM::updateBucket(int logicalIndex, const Bucket &newBucket) {
    int physicalIndex = toPhysicalIndex(logicalIndex);
    if (physicalIndex < 0 || physicalIndex >= heap.size()) {
        throw std::out_of_range("Invalid bucket index");
    }
    heap[physicalIndex] = newBucket;
}

void ORAM::updateBucketAtLevel(int level, int index_in_level, const Bucket &newBucket) {
    int levelStart = (1 << level) - 1;
    int levelCount = (1 << level);
    if (index_in_level < 0 || index_in_level >= levelCount) {
        throw std::out_of_range("Bucket index out of range for the specified level");
    }
    int normalIndex = levelStart + index_in_level;
    updateBucket(normalIndex, newBucket);
}

vector<Bucket> ORAM::try_buckets_at_level(int level, int leaf, int range_power) {
    int physical_leaf = leafToPhysicalIndex(leaf);
    int level_start = (1 << level) - 1;
    int level_count = min((1 << level), num_buckets - level_start);
    
    int height = ceil(log2(num_buckets + 1)) - 1;
    int leaf_level = height - 1;
    
    int physical_index;
    
    if (level == leaf_level) {
        physical_index = physical_leaf - level_start;
    } else {
        int logical_leaf = toNormalIndex(physical_leaf);
        int logical_ancestor = logical_leaf;
        for (int i = 0; i < leaf_level - level; i++) {
            logical_ancestor = (logical_ancestor - 1) / 2;
        }
        int physical_ancestor = toPhysicalIndex(logical_ancestor);
        physical_index = physical_ancestor - level_start;
    }
    vector<Bucket> result;
    for (int i = 0; i < (1 << range_power); i++) {
        int idx = (physical_index + i) % level_count;
        result.push_back(heap[level_start + idx]);
    }
    return result;
}

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

block ORAM::writeBlockToPath(const block &b, int logicalLeaf) {
    vector<int> path_indices = getpathindicies_ltor(logicalLeaf);
    for (int bucketIndex : path_indices) {
        if (heap[bucketIndex].add_block_in_tree(b,encryption_key)) {
            return block();
        }
    }
    return b;
}
