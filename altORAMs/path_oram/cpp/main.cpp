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
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

int main() {
    // initial parameters
    int num_buckets_low = 1024; // lower bound for buckets - fixed
    cout << "num buckets: " << num_buckets_low << endl;
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

    // Define the range query sizes using exponents: 2^1, 2^4, 2^10, and 2^14.
    vector<int> exponents = {1, 4, 10};

    for (int exp : exponents) {
        int numBlocksExpected = 1 << exp;  // Calculate 2^exp
        int startIdx = 0;
        int endIdx = numBlocksExpected - 1;
        cout << "Performing range query for " << numBlocksExpected 
             << " blocks (from " << startIdx << " to " << endIdx << ")" << endl;
        
        auto start = high_resolution_clock::now();
        vector<block> range_result = client.range_query(startIdx, endIdx);
        auto end = high_resolution_clock::now();
        double duration_seconds = duration_cast<duration<double>>(end - start).count();
        
        cout << "Total time for range query: " << duration_seconds << " seconds" << endl;
        if (!range_result.empty()) {
            double timePerBlock = duration_seconds / static_cast<double>(range_result.size());
            cout << "Average time per block: " << timePerBlock << " seconds" << endl;
        } else {
            cout << "No blocks returned by range query." << endl;
        }
        cout << "---------------------------------------------------" << endl;
    }
    
    return 0;
}
