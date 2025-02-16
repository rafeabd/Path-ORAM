#include "../include/client.h"
#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/bst.h"
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
    int num_buckets_low = 500; // lower bound for buckets - fixed
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
    BucketHeap oram_tree(num_buckets, bucket_capacity, encryptionKey);
    // init server
    Server server(num_buckets_low, bucket_capacity, move(oram_tree));
    // init client
    Client client(num_buckets_low, &server, encryptionKey);

    // Read dataset file and write blocks from it
    string datasetPath = "/Users/rabdulali/Desktop/Path-ORAM/tests/2^10.txt";
    ifstream infile(datasetPath);
    string line;
    while(getline(infile, line)) {
        istringstream iss(line);
        string id_str, data;
        if(getline(iss, id_str, ',') && getline(iss, data)) {
            int id = stoi(id_str);
            data.erase(0, data.find_first_not_of(" \t"));
            client.access(1, id, data);
        }
    }
    infile.close();

    // Example read accesses:
    cout << "Reading Blocks:" << endl;
    for (int i = 0; i < 1024; i++) {
        client.access(0, i, "");
    }

    /*
    cout << "range query" << endl;
    vector<block> range_result = client.range_query(684,1988);
    for (block &b : range_result) {
        b.print_block();
    }
    */

    cout << "printing server view" << endl;
    //server.printHeap();
    cout << "Printing stash:" << endl;
    //client.print_stash();

    return 0;
}
