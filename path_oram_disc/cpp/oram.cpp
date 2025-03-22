#include "../include/oram.h"
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

#define bucket_char_size 16384

BucketHeap::BucketHeap(int numBuckets, int bucketCapacity, const vector<unsigned char>& encKey)
    : bucketCapacity(bucketCapacity), encryptionKey(encKey)
{
    this->file_path = "tree/oram";
    this->tree_file.open(file_path, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);

    Bucket bucket(bucketCapacity);
    for (int i = 0; i < numBuckets; i++) {
        
        // this is dumb but need to clear buckets after they have been initialized - because we are adding dummt bu
        bucket.clear();
        //add encrypted dummy blocks to oram buckets
        for (int j = 0; j < bucketCapacity; j++) {
            block dummyBlock(-1, -1, "dummy", true);
            dummyBlock = encryptBlock(dummyBlock, encryptionKey);
            bucket.startaddblock(dummyBlock);
        }

        tree_file << serialize_bucket(bucket);    
    }
    //flushCache();
    //cout << "done" << endl;
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

Bucket BucketHeap::getBucket(int index) {
    //cout << index << endl;
    tree_file.clear();
    const std::streamoff offset = index * bucket_char_size;
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
    //result.print_bucket();
    return result;
}

void BucketHeap::updateBucket(int index, Bucket& bucket) {
    Bucket bucket_to_update = decrypt_bucket(bucket, encryptionKey);

    tree_file.clear();
    const std::streamoff offset = index * bucket_char_size;
    tree_file.seekp(offset, std::ios::beg);
    if (!tree_file) {
        reopenFile();
        if(!tree_file){
            throw std::runtime_error("Seek failed.");
        }
    }

    bucket_to_update = encrypt_bucket(bucket_to_update, encryptionKey);
    std::string bucket_data = serialize_bucket(bucket);
    //cout << bucket_data.size() << endl;
    //if (bucket_data.size() != 16384){
    //    cout << "bucket that is the wrong size" << endl;
    //    bucket.print_bucket();
    //}
    tree_file.write(bucket_data.data(), bucket_char_size);

    if (!tree_file) {
        throw std::runtime_error("Write failed.");
    }
    //flushCache();
}


// Returns a vector containing the path from a leaf bucket to the root.
vector<block> BucketHeap::getPathFromLeaf(int leafIndex) {
    vector<block> path;  // Vector to store blocks along the path.
    int current = leafIndex;  // Start at the given leaf index.
    while (true) {
        vector<block> blocks = getBucket(current).getBlocks();
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
        path.push_back(getBucket(current));
        if (current == 0) break;  
        current = parent(current);
    }
    reverse(path.begin(), path.end());

    //very stupid, hurts performance, but needed right now
    for(Bucket &bucket : path){
        bucket = decrypt_bucket(bucket,encryptionKey);
    }

    for (Bucket &bucket : path){
        bucket = encrypt_bucket(bucket, encryptionKey);
    }
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
    updateBucket(index, newBucket);
}

void BucketHeap::reopenFile() {
    if (tree_file.is_open()) {
        tree_file.close();
    }
    tree_file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!tree_file.is_open()) {
        throw std::runtime_error("Failed to reopen file");
    }
}

void BucketHeap :: flushCache() {
    tree_file.flush();
    system("sync");
}
