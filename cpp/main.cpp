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
    int num_blocks = 10000; // actually lower bound for buckets, should fix
    int bucket_capacity = 4;
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

    // Read dataset file and write blocks from it
    string datasetPath = "/Users/rabdulali/Desktop/Path-ORAM/tests/2^20.txt";
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
    cout << "Trying to read blocks." << endl;
    client.access(0, 0, "").print_block();
    client.access(0, 999, "").print_block();

    //cout << "Printing stash:" << endl;
    //client.print_stash();

    return 0;
}
