#include "../include/oram.h"

#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

//init with random blocks.
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

// calculate the bit reverse of the number
int ORAM::bitReverse(int x, int bits) {
    int y = 0;
    for (int i = 0; i < bits; i++) {
        y = (y << 1) | (x & 1);
        x >>= 1;
    }
    return y;
}

// Convert a physical (bit-reversed-lexographic) index to its normal index
int ORAM::toNormalIndex(int physicalIndex) {
    // Get level of physical index
    int level = 0;
    int temp = physicalIndex + 1;
    while (temp >>= 1){
        level++;
    }
    int levelStart = (1 << level) - 1;

    // Get position in the level
    int pos_br = physicalIndex - levelStart;
    
    // Get bitreverse version - returns normal)
    int pos_normal = bitReverse(pos_br, level);
    return levelStart + pos_normal;
}

// Convert a normal index to its physical (bit-reversed-lexographic index)
int ORAM::toPhysicalIndex(int normalIndex) {
    int level = 0;
    int temp = normalIndex + 1;
    while (temp >>= 1){
        level++;
    }
    int levelStart = (1 << level) - 1;
    int pos_normal = normalIndex - levelStart;
    
    int pos_br = bitReverse(pos_normal, level);
    return levelStart + pos_br;
}

// Convert a leaf index (0 to N-1) to a physical index at the leaf level
int ORAM::leafToPhysicalIndex(int leaf) {
    // Calculate tree height
    int height = ceil(log2(num_buckets + 1));
    
    // Calculate level of leaf nodes (bottom level)
    int leafLevel = height - 1;
    
    // First leaf node is at index 2^(h-1) - 1 in normal indexing
    int firstLeafIndex = (1 << leafLevel) - 1;
    
    // Calculate normal index for this leaf
    int normalIndex = firstLeafIndex + leaf;
    
    // Convert to physical index
    return toPhysicalIndex(normalIndex);
}

int ORAM::parent(int i) {
    if (i == 0){
        return -1;
    }
    // Convert physical index to normal index
    int normal_index = toNormalIndex(i);
    // Compute parent's normal index
    int normal_parent = (normal_index - 1) / 2;
    // Convert back to physical index
    return toPhysicalIndex(normal_parent);
}

int ORAM::leftChild(int i) {
    // Convert physical index to normal index
    int normal_index = toNormalIndex(i);
    // Compute left child's normal index
    int normal_left = 2 * normal_index + 1;
    // Convert back to physical index
    return toPhysicalIndex(normal_left);
}

int ORAM::rightChild(int i) {
    // Convert physical index to normal index
    int normal_index = toNormalIndex(i);
    // Compute right child's normal index
    int normal_right = 2 * normal_index + 2;
    // Convert back to physical index
    return toPhysicalIndex(normal_right);
}

// Update a bucket based on the NORMAL INDEX
void ORAM::updateBucket(int normalIndex, const Bucket &newBucket) {
    int physicalIndex = toPhysicalIndex(normalIndex);
    if (physicalIndex < 0 || physicalIndex >= heap.size()) {
        throw std::out_of_range("Invalid bucket index");
    }
    heap[physicalIndex] = newBucket;
}

// Print the logical ORAM, this is the normal bst layout.
void ORAM::print_logical_oram() {
    for (int normalIndex = 0; normalIndex < heap.size(); normalIndex++) {
        int physicalIndex = toPhysicalIndex(normalIndex);
        cout << "Logical Bucket " << normalIndex 
             << " (Physical index: " << physicalIndex << ")" << endl;
        heap[physicalIndex].print_bucket();
    }
}


void ORAM::print_physical_oram(bool split_levels) {
    int index = 0;
    int level = 0;
    if (split_levels) {
        while (index < heap.size()) {
            int levelBucketCount = 1 << level; 
            for (int i = 0; i < levelBucketCount && index < heap.size(); i++) {
                cout << "Bucket " << index << endl;
                heap[index].print_bucket();
                index++;
            }
            cout << endl; 
            level++;
        }
    } else {
        for (int i = 0; i < heap.size(); i++) {
            cout << "Bucket " << i << endl;
            heap[i].print_bucket();
        }
    }
}

// Get path indices from leaf to root
vector<int> ORAM::getpathindicies_ltor(int leaf){
    vector<int> path_indices;
    
    // Convert leaf index to physical index
    int physical_leaf = leafToPhysicalIndex(leaf);
    cout << "Leaf " << leaf << " maps to physical index " << physical_leaf << endl;
    
    // Start with leaf and follow parent pointers
    int current = physical_leaf;
    while (current >= 0) {
        path_indices.push_back(current);
        current = parent(current);
    }
    
    cout << "Complete path from leaf to root: ";
    for (int idx : path_indices) {
        cout << idx << " ";
    }
    cout << endl;
    
    return path_indices;
}

// Get path indices from root to leaf
vector<int> ORAM::getpathindicies_rtol(int leaf){
    vector<int> path_indices = getpathindicies_ltor(leaf);
    reverse(path_indices.begin(), path_indices.end());
    
    cout << "Complete path from root to leaf: ";
    for (int idx : path_indices) {
        cout << idx << " ";
    }
    cout << endl;
    
    return path_indices;
}

Bucket ORAM::read_bucket(int logical_index){
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

vector<vector<Bucket>> ORAM::read_range(int leaf){
    vector<vector<Bucket>> result;
    
    cout << "Reading complete path from leaf " << leaf << " to root" << endl;
    
    // Get path from leaf to root
    vector<int> path_indices = getpathindicies_ltor(leaf);
    cout << "Path has " << path_indices.size() << " nodes" << endl;
    
    // For each node in the path, create a level in the result
    for (size_t i = 0; i < path_indices.size(); i++) {
        int physical_idx = path_indices[i];
        
        // Calculate the level based on the physical index
        int level = 0;
        int temp = physical_idx + 1;
        while (temp >>= 1) {
            level++;
        }
        
        cout << "Reading node at physical index " << physical_idx << " (level " << level << ")" << endl;
        
        // Create a vector with just this bucket
        vector<Bucket> level_buckets;
        level_buckets.push_back(heap[physical_idx]);
        result.push_back(level_buckets);
    }
    
    cout << "Read " << result.size() << " levels from the tree" << endl;
    for (size_t i = 0; i < result.size(); i++) {
        cout << "  Level " << i << ": " << result[i].size() << " buckets" << endl;
        
        // Debug: Print bucket contents
        for (size_t j = 0; j < result[i].size(); j++) {
            cout << "    Bucket " << j << " contents:" << endl;
            for (const block& blk : result[i][j].getBlocks()) {
                if (!blk.dummy) {
                    cout << "      Block ID: " << blk.id 
                         << ", Data: '" << blk.data << "'" << endl;
                }
            }
        }
    }
    
    return result;
}

//compute path labels
vector<int> ORAM::get_path_labels_mod(int leaf) {
    vector<int> path;
    int h = 0;
    while ((1 << h) - 1 < num_buckets) {
        h++;
    }
    for (int j = 0; j < h; j++) {
        int label = leaf % (1 << j); 
        path.push_back(label);
    }
    return path;
}

Bucket ORAM::get_bucket_at_level(int level, int index_in_level) {
    int levelStart = (1 << level) - 1;
    int levelCount = std::min((1 << level), num_buckets - levelStart);
    if (index_in_level < 0 || index_in_level >= levelCount) {
        throw std::out_of_range("Bucket index out of range for the specified level");
    }
    int physicalIndex = levelStart + index_in_level;
    return heap[physicalIndex];
}