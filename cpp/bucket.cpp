#include "../include/buckets.h"

// Constructor to initialize the bucket with dummy blocks
bucket::bucket(int bucket_size) {
    for (int i = 0; i < bucket_size; ++i) {
        blocks.push_back(block(-1, -1, "", true)); // Add dummy blocks
    }
}