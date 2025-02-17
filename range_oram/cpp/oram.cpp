#include "oram.h"
#include "bst.h"

ORAM::ORAM(int num_buckets, int bucket_capacity, vector<unsigned char> encryption_key, int max_range_size){
    for (int i = 0; i=log2(max_range_size); i++){
        BucketHeap oram_tree(num_buckets, bucket_capacity, encryption_key);
    }

}