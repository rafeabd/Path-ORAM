#include <iostream>
#include <vector>

#include "bst.h"

using namespace std;

class ORAM{
    private:
        vector<BucketHeap> oram;
    public:
        ORAM(int num_buckets, int bucket_capacity, vector<unsigned char> encryption_key, int max_range_size);
};