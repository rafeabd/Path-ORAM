#include "../include/client.h"
#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/bst.h"
#include "../include/encryption.h"
#include <iostream>
#include <cmath>
#include <memory>
#include <utility>

using namespace std;

int main() {
    // initial paramameters
    int num_blocks = 1000; //actually lower bound for buckets, should fix
    int bucket_capacity = 4;
    //int L = ceil(num_blocks/4);
    int L = ceil(log2(num_blocks));
    
    // num buckets in oram
    int num_buckets = (1 << (L + 1)) - 1;
    cout << "Initializing oram with " << num_buckets << " buckets." << endl;

    // generate encryption key
    vector<unsigned char> encryptionKey = generateEncryptionKey(32);

    // init oram
    BucketHeap oram_tree(num_buckets, bucket_capacity, encryptionKey);
    // init server
    Server server(num_blocks, bucket_capacity, move(oram_tree));
    // init client
    Client client(num_blocks, &server, encryptionKey);
    //server.printHeap();
    
    cout << "Writing blocks." << endl;
    for (int i = 0; i < 2000; i++) {
        string data = to_string(i);
        client.access(1, i, data);
    }
    

    //cout << "reading blocks" << endl;
    cout << "trying to read" << endl;
    client.access(0,48,"").print_block();
    client.access(0,86,"").print_block();
    client.access(0,1834,"").print_block();
    client.access(0,1834,"").print_block();
    client.access(0,1834,"").print_block();
    client.access(0,1834,"").print_block();
    client.access(0,1834,"").print_block();
    client.access(0,1834,"").print_block();

    //server.printHeap();
    //server.printHeap();

    //cout << "printing oram" << endl;
    //server.printHeap();
    cout << "printing stash" << endl;
    client.print_stash();

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
