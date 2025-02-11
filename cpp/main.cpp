#include "../include/client.h"
#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/bst.h"
#include <iostream>
#include <cmath>
#include <memory>
#include <utility>

using namespace std;

int main() {
    // initial paramameters
    int num_blocks = 16;
    int bucket_capacity = 4;
    int L = ceil(log2(num_blocks));  
    
    // num buckets in oram
    int num_buckets = (1 << (L + 1)) - 1;
    cout << "Initializing ORAM with " << num_buckets << " buckets." << endl;

    // init oram
    BucketHeap oram_tree(num_buckets, bucket_capacity);
    
    // init server
    Server server(num_blocks, bucket_capacity, move(oram_tree));
    
    // init client
    Client client(num_blocks, &server);
    
    cout << "Writing block id 3." << endl;
    client.access(1, 3, "OWAM!");
    
    cout << "Reading block id 3." << endl;
    block result = client.access(0, 3, "");
    cout << "Block id: " << result.id << ", Data: " << result.data << endl;

    return 0;
}
