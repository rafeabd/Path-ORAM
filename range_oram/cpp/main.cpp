#include "../include/client.h"
#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/oram.h"
#include "../include/encryption.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <memory>
#include <utility>

using namespace std;

int main() {
    // initial parameters
    int num_buckets_low = 7; // lower bound for buckets - fixed
    cout << "num buckets" << num_buckets_low << endl;
    int bucket_capacity = 4;
    int L = ceil(log2(num_buckets_low));
    
    // num buckets in oram
    int num_buckets = (1 << (L + 1)) - 1;
    cout << "num_bucket2: " << num_buckets << endl;
    cout << "Initializing oram with " << num_buckets << " buckets." << endl;

    // generate encryption key
    vector<unsigned char> encryptionKey = generateEncryptionKey(32);

    // init oram
    ORAM oram_tree(num_buckets, bucket_capacity, encryptionKey);
    for (int i = 0; i < num_buckets; i++) {
        Bucket b(bucket_capacity);
        b.addBlock(block(i, i, to_string(i), true));
        oram_tree.updateBucket(i, b);
    }

    cout << "Physical ORAM:" << endl;
    oram_tree.print_physical_oram();
    cout << "Logical ORAM:" << endl;
    oram_tree.print_logical_oram();

    return 0;
}
