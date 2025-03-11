#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <unordered_set>
#include <cmath>
#include "../include/client.h"
#include "../include/server.h"
#include "../include/oram.h"

using namespace std;

int main() {
    // Number of blocks to test
    int num_blocks = 1000;
    
    // Query range parameters
    int query_range = 1000;  // Default range size for queries
    
    // Calculate appropriate ORAM parameters
    // Buckets should be 2^i-1 where i makes it larger than num_blocks
    int power = 1;
    while ((1 << power) - 1 < num_blocks) {
        power++;
    }
    int num_buckets = (1 << power) - 1;
    
    // Other parameters
    int bucket_capacity = 4;
    int max_range = (1 << power) + 1;  // 2^i+1 where i is the power used for buckets

    //max_range = 5;

    cout << "=== ORAM ALGORITHM TESTING ===" << endl;
    cout << "Testing with " << num_blocks << " blocks" << endl;
    cout << "Query range size: " << query_range << endl;
    cout << "Initializing client with " << num_buckets << " buckets of size "
         << bucket_capacity << ", max range " << max_range << endl;
    
    // Create test data
    vector<pair<int, string>> data_to_add;
    for (int i = 0; i < num_blocks; i++) {
        string data = "Test " + to_string(i);
        data_to_add.push_back(make_pair(i, data));
    }

    // Initialize client with the data
    Client client(data_to_add, bucket_capacity, max_range);
    
    //cout << "Initial state of logical trees:" << endl;
    //for (int i = 0; i < client.num_trees; i++) {
    //    client.printLogicalTreeState(i, 10, 1); // Print first 10 elements of each tree
    //}
    //client.print_position_maps();
    
    /*
    auto result = client.simple_access(5, 1, 0, {});
    for (block b: result){
        cout << "result block" << endl;
        b.print_block();
    }
    */
    
    cout << "\nAccessing blocks by range to verify data integrity..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    // Calculate how many range queries we need
    int num_range_queries = ceil((double)num_blocks / query_range);
    
    // Track successful retrievals
    int successful_retrievals = 0;
    unordered_set<int> accessed_ids;
    
    // Access blocks by ranges
    for (int range_idx = 0; range_idx < num_range_queries; range_idx++) {
        int start_id = range_idx * query_range;
        
        cout << "  Querying range starting at block " << start_id 
             << " (range size: " << query_range << ")" << endl;
             
        auto result = client.simple_access(start_id, query_range, 0, {});
        
        // Check each block in the result
        cout << "  Retrieved " << result.size() << " blocks in this range" << endl;
        
        for (auto& blk : result) {
            int id = blk.id;
            //cout << "block retrieved: ";
            //blk.print_block();
            
            // Check if this is a valid block ID we care about
            if (id >= 0 && id < num_blocks) {
                string expected_data = "Test " + to_string(id);
                
                if (blk.data == expected_data) {
                    successful_retrievals++;
                    accessed_ids.insert(id);
                    //cout << "    Block ID " << id << ": Data correct" << endl;
                } else {
                    cout << "    ERROR: Block ID " << id << " has incorrect data. Expected: '" 
                         << expected_data << "', Got: '" << blk.data << "'" << endl;
                }
            }
        }
        
        // Print progress
        cout << "  Progress: " << min((range_idx + 1) * query_range, num_blocks) << "/" << num_blocks 
             << " blocks processed, " << successful_retrievals << " successful" << endl;
        cout << endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    // Print final results
    cout << "\nRetrieval results:" << endl;
    cout << "  Total blocks: " << num_blocks << endl;
    cout << "  Successfully retrieved: " << successful_retrievals << endl;
    cout << "  Success rate: " << (double)successful_retrievals / num_blocks * 100 << "%" << endl;
    
    // Check if any blocks were missed
    if (successful_retrievals < num_blocks) {
        cout << "\nMissing blocks:" << endl;
        for (int i = 0; i < num_blocks; i++) {
            if (accessed_ids.find(i) == accessed_ids.end()) {
                cout << "Block ID: " << i;
                cout << ", ";
            }
        }
        cout << endl;
    } else {
        cout << "\nAll blocks were successfully retrieved!" << endl;
    }

    //cout << "printing final tree state: " << endl;
    //for (int i = 0; i < client.num_trees; i++) {
    //    client.printLogicalTreeState(i, 10, 1); // Print first 10 elements of each tree
    //}

    std::cout << "Total reading time: " << elapsed_seconds.count() << "s\n";
    cout << "\n=== All tests completed ===" << endl;
    
    return 0;
}