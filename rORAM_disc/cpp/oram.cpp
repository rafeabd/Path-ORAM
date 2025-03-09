#include "../include/oram.h"
#include "../include/encryption.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <set>

using namespace std;

#define bucket_char_size 1280

ORAM::ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length, string file) {
    this->encryptionKey = encryptionKey;
    this->bucketCapacity = bucketCapacity;
    this->num_buckets = numBuckets;
    this->range_length = range_length;
    this->global_counter = 0;

    file = "trees/" + file;
    this->tree_file.open(file, std::ios::in | std::ios::out | std::ios::trunc);
    
    for (int i = 0; i < numBuckets; i++) {
        Bucket bucket_to_add;
        for (int j = 0; j < 4; j++){
            block block_to_add = block();
            bucket_to_add.addBlock(encryptBlock(block_to_add,encryptionKey));
        }
        tree_file << serialize_bucket(bucket_to_add);
    }
    tree_file.flush();
}

ORAM::~ORAM() {
    if (tree_file.is_open()) {
        tree_file.close();
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
    // Calculate the byte offset for the desired node.
    //cout << "logical index in read_bucket" << logical_index << endl;
    const std::streamoff offset = toPhysicalIndex(logical_index) * bucket_char_size;
    tree_file.seekg(offset, std::ios::beg);
    if (!tree_file) {
        throw std::runtime_error("Seek failed.");
    }
    char buffer[bucket_char_size];
    tree_file.read(buffer, bucket_char_size);
    //cout << "in read bucket" << endl;
    //cout << tree_file.gcount() << endl;
    if (tree_file.gcount() != bucket_char_size) {
        throw std::runtime_error("Failed to read the full bucket.");
    }
    string bucket_data(buffer, bucket_char_size);
    Bucket result = deserialize_bucket(bucket_data);
    return result;
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
        
        // Read the bucket from disk using its logical index.
        Bucket b = read_bucket(normalIndex);
        buckets.push_back(b);
        
        // Create an empty bucket. Here we fill it with bucketCapacity dummy blocks.
        Bucket emptyBucket(bucketCapacity);
        for (int i = 0; i < bucketCapacity; i++) {
            block dummy = block();
            emptyBucket.addBlock(encryptBlock(dummy, encryptionKey));
        }
        // Write the empty bucket back to disk.
        updateBucket(normalIndex, emptyBucket);
        
    }
    return buckets;
}

void ORAM::updateBucket(int logicalIndex, const Bucket &newBucket) {
    const std::streamoff offset = toPhysicalIndex(logicalIndex) * bucket_char_size;
    tree_file.seekp(offset, std::ios::beg);
    if (!tree_file) {
        throw std::runtime_error("Seek failed.");
    }

    std::string bucket_data = serialize_bucket(newBucket);
    tree_file.write(bucket_data.data(), bucket_char_size);

    if (!tree_file) {
        throw std::runtime_error("Write failed.");
    }
    tree_file.flush();
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
        result.push_back(read_bucket(level_start+idx));
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

block ORAM::writeBlockToPath(const block &b, int logicalLeaf, vector<unsigned char> key) {
    vector<int> path_indices = getpathindicies_ltor(logicalLeaf);
    for (int physIndex : path_indices) {
        // Convert the physical index back to its logical index.
        //int logicalIndex = toNormalIndex(physIndex);
        //Bucket currentBucket = read_bucket(logicalIndex);
        Bucket currentBucket = read_bucket(physIndex);

        //we need to decrypt the bucket because otherwise all blocks will look real.

        for (block &blocks_in_bucket: currentBucket.blocks){
            blocks_in_bucket = decryptBlock(blocks_in_bucket, key);
        }
        // Encrypt the block before inserting.
        if (currentBucket.addBlock(b)) {
            for (block &blocks_in_bucket: currentBucket.blocks){
                blocks_in_bucket = encryptBlock(blocks_in_bucket, key);
            }
            //updateBucket(logicalIndex, currentBucket);
            updateBucket(physIndex,currentBucket);
            // Return an empty (dummy) block to indicate that the write was successful.
            return block(-1, "", true, vector<int>{});
        }
    }
    // If the block could not be written to any bucket along the path,
    // return it so it can be kept in the stash.
    return b;
}

