#include "../include/oram.h"
#include "../include/encryption.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <set>

using namespace std;
ORAM::ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length) {
    this->bucketCapacity = bucketCapacity;
    this->num_buckets = numBuckets;
    this->range_length = range_length;
    this->global_counter = 0;
    heap.reserve(numBuckets);
    ofstream MyFile(heap);


    for (int i = 0; i < numBuckets; i++) {
        heap.push_back(Bucket(bucketCapacity));
    }
}

//serialize and deserialize buckets into strings;
string ORAM::serialize_bucket(Bucket bucket) {
    string serialized_bucket;
    for (block b : bucket.getBlocks()){
        serialized_bucket += serializeBlock(b);
    }
    string cipherHex = serialized_bucket;
    return serialize_bucket;
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
    int height = ceil(log2(num_buckets + 1))-1;
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

int ORAM::leftChild(int i) {
    int normal_index = toNormalIndex(i);
    int normal_left = 2 * normal_index + 1;
    return toPhysicalIndex(normal_left);
}

int ORAM::rightChild(int i) {
    int normal_index = toNormalIndex(i);
    int normal_right = 2 * normal_index + 2;
    return toPhysicalIndex(normal_right);
}

Bucket ORAM::read_bucket(int logical_index) {
    return heap[toPhysicalIndex(logical_index)];
}

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

vector<int> ORAM::getpathindicies_rtol(int leaf) {
    vector<int> path_indices = getpathindicies_ltor(leaf);
    reverse(path_indices.begin(), path_indices.end());
    return path_indices;
}

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

void ORAM::updateBucket(int logicalIndex, const Bucket &newBucket) {
    int physicalIndex = toPhysicalIndex(logicalIndex);
    if (physicalIndex < 0 || physicalIndex >= heap.size()) {
        throw std::out_of_range("Invalid bucket index");
    }
    heap[physicalIndex] = newBucket;
}

vector<Bucket> ORAM::readBucketsAtLevel(int level, int start_index, int count) {
    int levelStart = (1 << level) - 1;
    int levelCount = 1 << level;
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
    }
    return buckets;
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
        // Clear the bucket by replacing with an empty dummy bucket
        heap[physicalIndex] = Bucket(bucketCapacity);
    }
    return buckets;
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

vector<Bucket> ORAM::simple_buckets_at_level(int level, int leaf, int range_power) {
    int physical_leaf = toPhysicalIndex(leaf);
    int physical_index = physical_leaf % (1 << level);
    int level_start = (1 << level) - 1;
    int level_count = (1 << level);
    vector<Bucket> result;
    for (int i = 0; i < (1 << range_power); i++) {
        int idx = (physical_index + i) % level_count;
        cout << "idx: " << idx << endl;
        result.push_back(heap[level_start + idx]);
    }
    return result;
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

void ORAM::simple_update_bucket(int level, int inx_in_level, Bucket updated_bucket) {
    int level_start = (1 << level) - 1;
    heap[level_start + inx_in_level] = updated_bucket;
}

int ORAM::simple_toPhysical(int index, int level) {
    if (level == 0) {
        return 0;
    }
    int levelStart = (1 << level) - 1;
    int pos_br = bitReverse(index, level);
    return levelStart + pos_br;
}

block ORAM::writeBlockToPath(const block &b, int logicalLeaf) {
    vector<int> path_indices = getpathindicies_ltor(logicalLeaf);
    for (int bucketIndex : path_indices) {
        if (heap[bucketIndex].addBlock(b)) {
            return block();
        }
    }
    return b;
}