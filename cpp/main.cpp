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
    int num_blocks = 5;
    int bucket_capacity = 4;
    //int L = ceil(num_blocks/4);
    int L = ceil(log2(num_blocks));
    
    // num buckets in oram
    int num_buckets = (1 << (L + 1)) - 1;
    cout << "Initializing oram with " << num_buckets << " buckets." << endl;

    // init oram
    BucketHeap oram_tree(num_buckets, bucket_capacity);
    // init server
    Server server(num_blocks, bucket_capacity, move(oram_tree));
    // init client
    Client client(num_blocks, &server);
    //server.printHeap();
    
    cout << "Writing blocks." << endl;
    for (int i = 0; i < 20; i++) {
        string data = to_string(i);
        client.access(1, i, data);
    }

    //cout << "reading blocks" << endl;
    //client.access(0,600,"").print_block();

    //cout << "printing oram" << endl;
    //server.printHeap();
    //cout << "printing stash" << endl;
    //client.print_stash();

    /*
    client.access(1, 0, "first block");
    cout << "printing oram" << endl;
    server.printHeap();
    cout << "printing stash" << endl;
    client.print_stash();
    */

    //client.access(0, 3, "").print_block();

    //client.print_stash();
    //server.printHeap();

    return 0;
}
