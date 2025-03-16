#include "../include/oram.h"
#include "../include/encryption.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <set>
#include <unistd.h>
#include <fcntl.h>  // Add this at the top with other includes

using namespace std;

#define bucket_char_size 16384

ORAM::ORAM(int numBuckets, int bucketCapacity, const vector<unsigned char>& encryptionKey, int range_length, string file) {
    this->encryptionKey = encryptionKey;
    this->bucketCapacity = bucketCapacity;
    this->num_buckets = numBuckets;
    this->range_length = range_length;
    this->global_counter = 0;
    this->file_path = "trees/" + file;
    this->tree_file.open(file_path, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    
    for (int i = 0; i < numBuckets; i++) {
        tree_file << serialize_bucket(encrypt_bucket(Bucket(),encryptionKey));
    }
    //tree_file.flush();
    //flushCache();
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
    int height = log2(num_buckets + 1);  
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

    tree_file.clear();
    const std::streamoff offset = toPhysicalIndex(logical_index) * bucket_char_size;
    tree_file.seekg(offset, std::ios::beg);
    if (!tree_file) {
        reopenFile();
        if(!tree_file){
            throw std::runtime_error("Seek failed.");
        }
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
    //flushCache();
    return result;
}

Bucket ORAM::read_bucket_physical(int physicalIndex) {
    // Calculate the byte offset for the desired node.
    //cout << "logical index in read_bucket" << logical_index << endl;
    tree_file.clear();
    const std::streamoff offset = physicalIndex * bucket_char_size;
    tree_file.seekg(offset, std::ios::beg);
    if (!tree_file) {
        reopenFile();
        if(!tree_file){
            throw std::runtime_error("Seek failed.");
        }
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
    //flushCache();
    return result;
}



vector<Bucket> ORAM::readBucketsAndClear(int level, int start_index, int count) {
    tree_file.clear();
    int levelStart = (1 << level) - 1;
    // No need to recalculate the level count, just use 2^level for a complete tree
    int levelCount = (1 << level);
    
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
        // Write the empty bucket back to disk.
        updateBucket(normalIndex, encrypt_bucket(Bucket(bucketCapacity), encryptionKey));
    }
    
    return buckets;
}

void ORAM::updateBucket(int logicalIndex, const Bucket &newBucket) {
    tree_file.clear();
    const std::streamoff offset = toPhysicalIndex(logicalIndex) * bucket_char_size;
    tree_file.seekp(offset, std::ios::beg);
    if (!tree_file) {
        reopenFile();
        if(!tree_file){
            throw std::runtime_error("Seek failed.");
        }
    }

    std::string bucket_data = serialize_bucket(newBucket);
    tree_file.write(bucket_data.data(), bucket_char_size);

    if (!tree_file) {
        throw std::runtime_error("Write failed.");
    }
    //flushCache();
}

void ORAM::updateBucketForInitialization(int logicalIndex, const Bucket &newBucket) {
    tree_file.clear();
    const std::streamoff offset = toPhysicalIndex(logicalIndex) * bucket_char_size;
    tree_file.seekp(offset, std::ios::beg);
    if (!tree_file) {
        reopenFile();
        if(!tree_file){
            throw std::runtime_error("Seek failed.");
        }
    }

    std::string bucket_data = serialize_bucket(newBucket);
    tree_file.write(bucket_data.data(), bucket_char_size);

    if (!tree_file) {
        throw std::runtime_error("Write failed.");
    }
}

void ORAM::updateBucketAtLevel(int level, int index_in_level, const Bucket &newBucket) {
    int levelStart = (1 << level) - 1;
    int levelCount = (1 << level);
    
    // Ensure the index is within bounds
    if (index_in_level < 0 || index_in_level >= levelCount) {
        throw std::out_of_range("Bucket index out of range for the specified level");
    }
    
    int normalIndex = levelStart + index_in_level;
    // Ensure we don't try to update beyond the tree bounds
    if (normalIndex >= num_buckets) {
        throw std::out_of_range("Bucket index exceeds tree size");
    }
    
    updateBucket(normalIndex, newBucket);
}

vector<Bucket> ORAM::try_buckets_at_level(int level, int leaf, int range_power) {
    int physical_leaf = leafToPhysicalIndex(leaf);
    //cout << "physical_leaf: " << physical_leaf << endl;
    int level_start = (1 << level) - 1;
    int level_count = min((1 << level), num_buckets - level_start);
    
    // Calculate height directly from num_buckets which is guaranteed to be 2^h-1
    int height = log2(num_buckets + 1);
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
        //cout << "physical index getting read" << level_start+idx << endl;
        result.push_back(read_bucket_physical(level_start+idx));
    }
    
    return result;
}

vector<int> ORAM::getpathindicies_ltor(int leaf) {
    vector<int> path_indices;
    int physical_leaf = leafToPhysicalIndex(leaf);
    int current = physical_leaf;
    
    while (current >= 0) {
        int logical_index = toNormalIndex(current);
        path_indices.push_back(logical_index); 
        current = parent(current);
    }
    
    return path_indices;
}

block ORAM::writeBlockToPath(const block &b, int logicalLeaf, vector<unsigned char> key) {
    //cout << "writingblocktopath" << endl;
    vector<int> path_indices = getpathindicies_ltor(logicalLeaf);
    for (int logicalIndex : path_indices) {
        Bucket currentBucket = read_bucket(logicalIndex);

        // Decrypt the bucket blocks
        for (block &blocks_in_bucket: currentBucket.blocks){
            blocks_in_bucket = decryptBlock(blocks_in_bucket, key);
        }
        
        // Try to add the block to this bucket
        if (currentBucket.addBlock(b)) {
            // Re-encrypt all blocks
            for (block &blocks_in_bucket: currentBucket.blocks){
                blocks_in_bucket = encryptBlock(blocks_in_bucket, key);
            }
            
            // Update the bucket
            updateBucketForInitialization(logicalIndex, currentBucket);
            
            // Return an empty (dummy) block to indicate success
            return block(-1, "", true, vector<int>{});
        }
    }
    
    // Block couldn't be added to any bucket in the path
    return b;
}

void ORAM::reopenFile() {
    if (tree_file.is_open()) {
        tree_file.close();
    }
    tree_file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!tree_file.is_open()) {
        throw std::runtime_error("Failed to reopen file");
    }
}

void ORAM::flushCache() {
    tree_file.flush();
    system("sync");
}
