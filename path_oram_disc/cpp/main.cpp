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
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "=== PATH-ORAM RANGE QUERY PERFORMANCE TEST ===" << endl;
    
    // Initial parameters

    //Set this to the total size of your database
    const int num_buckets_low = pow(2,10); 

    int bucket_capacity = 4;
    int L = ceil(log2(num_buckets_low));
    
    // Calculate actual number of buckets in ORAM
    int num_buckets = (1 << (L + 1)) - 1;
    
    cout << "Dataset parameters:" << endl;
    cout << "  Initial buckets: 2^" << log2(num_buckets_low) << " = " << num_buckets_low << endl;
    cout << "  Total buckets in ORAM: " << num_buckets << endl;
    cout << "  Bucket capacity: " << bucket_capacity << endl;
    
    // Generate encryption key
    cout << "Generating encryption key... ";
    vector<unsigned char> encryptionKey = generateEncryptionKey(64);
    cout << "done." << endl;

    // Initialize ORAM components
    cout << "Initializing ORAM system... ";
    BucketHeap oram_tree(num_buckets, bucket_capacity, encryptionKey);
    Server server(num_buckets_low, bucket_capacity, move(oram_tree));
    Client client(num_buckets_low, &server, encryptionKey);
    cout << "done." << endl;

    // Read dataset file and load data
    
    //Update this file path for your computer, and ensure it's for the correct database
    string datasetPath = "/c/Users/Documents/Path-ORAM/tests/2^10.txt"; //update it for your file path

    cout << "Loading dataset from: " << datasetPath << endl;
    
    ifstream infile(datasetPath);
    if (!infile) {
        cerr << "ERROR: Could not open dataset file!" << endl;
        return 1;
    }
    
    // Count loaded blocks for progress reporting
    int blocks_loaded = 0;
    int progress_interval = 100; // Show progress every 100 blocks
    
    cout << "Writing blocks to ORAM... ";
    string line;
    while(getline(infile, line)) {
        istringstream iss(line);
        string id_str, data;
        if(getline(iss, id_str, ',') && getline(iss, data)) {
            int id = stoi(id_str);
            data.erase(0, data.find_first_not_of(" \t"));
            client.access(1, id, data);
            
            blocks_loaded++;
            if (blocks_loaded % progress_interval == 0) {
                cout << blocks_loaded << " ";
                cout.flush();
            }
        }
    }
    cout << "done." << endl;
    cout << "Total blocks loaded: " << blocks_loaded << endl << endl;
    infile.close();

    // Define the range query sizes using exponents: 2^1, 2^4, 2^10
    vector<int> exponents = {1,2,3,4,5,6,7,8,9,10};
    
    // Store results for summary table
    struct QueryResult {
        int range_size;
        double total_time;
        double avg_time_per_block;
        int blocks_retrieved;
    };
    vector<QueryResult> results;

    cout << "=== RUNNING RANGE QUERY TESTS ===" << endl;
    cout << "Testing range query sizes: ";
    for (size_t i = 0; i < exponents.size(); i++) {
        cout << "2^" << exponents[i] << " (" << (1 << exponents[i]) << ")";
        if (i < exponents.size() - 1) cout << ", ";
    }
    cout << endl << endl;

    // Run tests for each range size
    for (size_t test_idx = 0; test_idx < exponents.size(); test_idx++) {
        int exp = exponents[test_idx];
        int range_size = 1 << exp;  // Calculate 2^exp
        
        cout << "\nTEST " << (test_idx + 1) << ": Range size = 2^" 
             << exp << " = " << range_size << " blocks" << endl;
             
        // Start from the beginning for this test
        int startIdx = 0;
        int endIdx = range_size - 1;
        
        cout << "Query starting at block " << startIdx 
             << ", ending at block " << endIdx 
             << " (range size = " << range_size << ")" << endl;

        // Time the query
        auto start = high_resolution_clock::now();
        vector<block> range_result = client.range_query(startIdx, endIdx);
        auto end = high_resolution_clock::now();
        
        // Calculate timing metrics
        double duration_seconds = duration_cast<duration<double>>(end - start).count();
        int blocks_retrieved = range_result.size();
        double avg_time_per_block = 0;
        
        if (!range_result.empty()) {
            avg_time_per_block = duration_seconds / static_cast<double>(blocks_retrieved);
        }
        
        // Store the results
        results.push_back({
            range_size,
            duration_seconds,
            avg_time_per_block,
            blocks_retrieved
        });

        // Print results for this range size
        cout << "  Retrieved " << blocks_retrieved << " blocks in " 
             << fixed << setprecision(6) << duration_seconds << " s" << endl;
             
        if (blocks_retrieved > 0) {
            cout << "  Average time per block: " 
                 << fixed << setprecision(8) << avg_time_per_block << " s/block" << endl;
        } else {
            cout << "  No blocks returned by range query." << endl;
        }
    }

    // Print summary table
    cout << "\n=== SUMMARY OF RESULTS ===" << endl;
    cout << "+---------------+---------------+---------------+---------------+" << endl;
    cout << "| Range Size    | Blocks        | Total Time    | Avg Time      |" << endl;
    cout << "| (power of 2)  | Retrieved     | (seconds)     | (s/block)     |" << endl;
    cout << "+---------------+---------------+---------------+---------------+" << endl;
    
    for (const auto& res : results) {
        int power = log2(res.range_size);
            
        cout << "| 2^" << setw(11) << left << power
             << "| " << setw(13) << left << res.blocks_retrieved
             << "| " << fixed << setprecision(6) << setw(13) << left << res.total_time
             << "| " << fixed << setprecision(8) << res.avg_time_per_block << " |" << endl;
    }
    cout << "+---------------+---------------+---------------+---------------+" << endl;

    cout << "\n=== All tests completed ===" << endl;
    return 0;
}