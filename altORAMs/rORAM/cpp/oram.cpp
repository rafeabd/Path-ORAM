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
#include <fcntl.h> 

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

vector<Bucket> ORAM::read_bucket_physical_consecutive(int physicalIndex, int range) {
    vector<Bucket> results;
    results.reserve(range); 
    if (range <= 0) return results;
    
    // Determine level information
    int level = 0;
    int temp = physicalIndex + 1;
    while (temp >>= 1) {
        level++;
    }
    int levelStart = (1 << level) - 1;
    int levelSize = (1 << level);
    int positionInLevel = physicalIndex - levelStart;
    
    range = min(range, levelSize);
    
    // Set number of buckets per read
    int maxChunkSize = 64; 
    
    int remaining = range;
    int currentPos = positionInLevel;
    
    while (remaining > 0) {
        int chunkSize = min(remaining, maxChunkSize);
        int continuousBucketsToRead = min(chunkSize, levelSize - currentPos);
        
        vector<char> buffer(continuousBucketsToRead * bucket_char_size);
        
        tree_file.clear();
        const std::streamoff offset = (levelStart + currentPos) * bucket_char_size;
        tree_file.seekg(offset, std::ios::beg);
        
        if (!tree_file) {
            reopenFile();
            if (!tree_file) {
                throw std::runtime_error("Seek failed at position " + to_string(offset));
            }
            tree_file.seekg(offset, std::ios::beg);
        }
        
        tree_file.read(buffer.data(), continuousBucketsToRead * bucket_char_size);
        
        if (tree_file.gcount() != continuousBucketsToRead * bucket_char_size) {
            throw std::runtime_error("Failed to read continuous bucket range. " 
                                    "Requested: " + to_string(continuousBucketsToRead * bucket_char_size) + 
                                    " bytes, Got: " + to_string(tree_file.gcount()) + " bytes");
        }
        
        for (int i = 0; i < continuousBucketsToRead; i++) {
            char* bucketStart = buffer.data() + (i * bucket_char_size);
            string bucket_data(bucketStart, bucket_char_size);
            results.push_back(deserialize_bucket(bucket_data));
        }
        
        // Update for next chunk
        remaining -= continuousBucketsToRead;
        currentPos = (currentPos + continuousBucketsToRead) % levelSize;
    }
    
    return results;
}



vector<Bucket> ORAM::readBucketsAndClear(int level, int start_index, int count) {
    int levelStart = (1 << level) - 1;
    int levelSize = (1 << level);
    
    vector<Bucket> results;
    
    // Collect unique logical indices
    vector<int> normalIndices;
    for (int t = start_index; t < start_index + count; ++t) {
        int offset = t % levelSize;
        int normalIndex = levelStart + offset;
        
        if (normalIndex < num_buckets && 
            find(normalIndices.begin(), normalIndices.end(), normalIndex) == normalIndices.end()) {
            normalIndices.push_back(normalIndex);
        }
    }
    
    // Convert to physical indices and sort
    vector<pair<int, int>> physicalIndices;
    for (int logicalIdx : normalIndices) {
        int physicalIdx = toPhysicalIndex(logicalIdx);
        physicalIndices.push_back({physicalIdx, logicalIdx});
    }
    
    sort(physicalIndices.begin(), physicalIndices.end());
    
    // Find contiguous ranges and read them in single operations
    for (size_t i = 0; i < physicalIndices.size();) {
        int startPhysical = physicalIndices[i].first;
        size_t rangeEnd = i + 1;
        
        // Find the end of the contiguous range
        while (rangeEnd < physicalIndices.size() && 
               physicalIndices[rangeEnd].first == physicalIndices[rangeEnd-1].first + 1) {
            rangeEnd++;
        }
        
        // Read the entire range once
        int contiguousCount = rangeEnd - i;
        vector<Bucket> rangeBuckets = read_bucket_physical_consecutive(startPhysical, contiguousCount);
        
        // Add buckets to results in their original logical order
        for (size_t j = i; j < rangeEnd; j++) {
            int localIndex = j - i;
            int originalLogicalIndex = physicalIndices[j].second;
            
            // Find position in results where this logical index should go
            auto pos = find_if(normalIndices.begin(), normalIndices.end(),
                              [originalLogicalIndex](int idx) { return idx == originalLogicalIndex; });
            if (pos != normalIndices.end()) {
                size_t resultIndex = pos - normalIndices.begin();
                if (resultIndex >= results.size()) {
                    results.resize(resultIndex + 1);
                }
                results[resultIndex] = rangeBuckets[localIndex];
            }
        }
        
        i = rangeEnd;
    }
    
    return results;
}

void ORAM::updateBucketsAtLevel(int level, const vector<pair<int, Bucket>>& indexBucketPairs) {
    if (indexBucketPairs.empty()) return;
    
    int levelStart = (1 << level) - 1;
    
    vector<pair<int, string>> serializedBuckets;
    serializedBuckets.reserve(indexBucketPairs.size());
    
    for (const auto& pair : indexBucketPairs) {
        int offsetInLevel = pair.first;
        int logicalIndex = levelStart + offsetInLevel;
        int physicalIndex = toPhysicalIndex(logicalIndex);
        
        string serialized = serialize_bucket(pair.second);
        serializedBuckets.emplace_back(physicalIndex, std::move(serialized));
    }
    
    //dumb sort
    std::sort(serializedBuckets.begin(), serializedBuckets.end(), 
             [](const pair<int, string>& a, const pair<int, string>& b) { 
                 return a.first < b.first; 
             });
    
    size_t i = 0;
    while (i < serializedBuckets.size()) {
        size_t rangeEnd = i + 1;
        while (rangeEnd < serializedBuckets.size() && 
               serializedBuckets[rangeEnd].first == serializedBuckets[rangeEnd-1].first + 1) {
            rangeEnd++;
        }
        
        int startPhysicalIndex = serializedBuckets[i].first;
        size_t rangeSize = rangeEnd - i;
        
        char* writeBuffer = new char[rangeSize * bucket_char_size];
        
        size_t bufferOffset = 0;
        for (size_t k = i; k < rangeEnd; k++) {
            memcpy(writeBuffer + bufferOffset, 
                   serializedBuckets[k].second.data(), 
                   bucket_char_size);
            bufferOffset += bucket_char_size;
        }
        
        // Single write
        tree_file.clear();
        const std::streamoff offset = startPhysicalIndex * bucket_char_size;
        
        // Perform the write with just one seek
        tree_file.seekp(offset, std::ios::beg);
        if (!tree_file) {
            tree_file.clear();
            reopenFile();
            tree_file.seekp(offset, std::ios::beg);
        }
        
        tree_file.write(writeBuffer, rangeSize * bucket_char_size);
        
        delete[] writeBuffer;
        
        i = rangeEnd;
    }

    //tree_file.flush();
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
    if (index_in_level < 0 || index_in_level >= levelCount) {
        throw std::out_of_range("Bucket index out of range for the specified level");
    }
    
    int normalIndex = levelStart + index_in_level;
    if (normalIndex >= num_buckets) {
        throw std::out_of_range("Bucket index exceeds tree size");
    }
    
    updateBucket(normalIndex, newBucket);
}

vector<Bucket> ORAM::try_buckets_at_level(int level, int leaf, int range_power) {
    int physical_leaf = leafToPhysicalIndex(leaf);
    int level_start = (1 << level) - 1;
    int level_count = min((1 << level), num_buckets - level_start);
    
    int height = log2(num_buckets + 1);
    int leaf_level = height - 1;
    
    int physical_index_within_level;
    
    if (level == leaf_level) {
        physical_index_within_level = physical_leaf - level_start;
    } else {
        int logical_leaf = toNormalIndex(physical_leaf);
        int logical_ancestor = logical_leaf;
        for (int i = 0; i < leaf_level - level; i++) {
            logical_ancestor = (logical_ancestor - 1) / 2;
        }
        int physical_ancestor = toPhysicalIndex(logical_ancestor);
        physical_index_within_level = physical_ancestor - level_start;
    }
    
    physical_index_within_level = physical_index_within_level % level_count;
    int absolute_physical_index = level_start + physical_index_within_level;
    
    // Read all buckets in one operation
    return read_bucket_physical_consecutive(absolute_physical_index, 1 << range_power);
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

        for (block &blocks_in_bucket: currentBucket.blocks){
            blocks_in_bucket = decryptBlock(blocks_in_bucket, key);
        }
        
        // Try to add the block to this bucket
        if (currentBucket.addBlock(b)) {
            for (block &blocks_in_bucket: currentBucket.blocks){
                blocks_in_bucket = encryptBlock(blocks_in_bucket, key);
            }
            
            updateBucketForInitialization(logicalIndex, currentBucket);
            
            // empty block for success
            return block(-1, "", true, vector<int>{});
        }
    }
    
    // failed, return block
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