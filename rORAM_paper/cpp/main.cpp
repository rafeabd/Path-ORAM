#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <unordered_set>
#include <cmath>
#include <chrono>
#include <iomanip>
#include "../include/client.h"
#include "../include/server.h"
#include "../include/oram.h"

using namespace std;


/*
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
*/






int main(int argc, char* argv[]) {
    // Fixed dataset size: 2^20 blocks
    const int dataset_size_power = 14;
    const int num_blocks = 1 << dataset_size_power; 
    
    // Range query sizes to test: 2^1, 2^4, 2^10, 2^14
    const vector<int> range_sizes = {

        1 << 1,  // 2^1 = 2
        1 << 2,
        1 << 3,
        1 << 4,  // 2^4 = 16
        //1 << 5,
        1 << 6,
        //1 << 7,
        1 << 8,
        //1 << 9,
        1 << 10, // 2^10 = 1024
        //1 << 11,
        1 << 12,
        //1 << 13,
        1 << 14,  // 2^14 = 16384
        //1 << 15

    };

    // Compute ORAM parameters based on the dataset size
    int power = 1;
    while ((1 << power) - 1 < num_blocks) {
        power++;
    }
    int num_buckets = (1 << power) - 1;
    int bucket_capacity = 4;
    
    // Find max range needed (the largest of our test range sizes)
    int max_range_power = 14; // 2^14 is our largest range
    int max_range = (1 << (max_range_power + 1)) + 1; // Adding buffer as in original code

    cout << "=== ORAM RANGE QUERY PERFORMANCE TEST ===" << endl;
    cout << "Dataset size: 2^" << dataset_size_power << " = " << num_blocks << " blocks" << endl;
    cout << "Testing range query sizes: ";
    for (size_t i = 0; i < range_sizes.size(); i++) {
        cout << "2^" << log2(range_sizes[i]) << " (" << range_sizes[i] << ")";
        if (i < range_sizes.size() - 1) cout << ", ";
    }
    cout << endl;
    
    cout << "Initializing client with " << num_buckets 
         << " buckets (capacity = " << bucket_capacity 
         << ", max range = " << max_range << ")" << endl << endl;
    
    // Create test data: each block is labeled "Test i" where i is its ID
    cout << "Creating test dataset... ";
    vector<pair<int, string>> data_to_add;
    data_to_add.reserve(num_blocks);
    
    // Show progress indicators for large datasets
    int progress_interval = num_blocks / 10; // Show progress at 10% intervals
    if (progress_interval < 1) progress_interval = 1;
    
    for (int idx = 0; idx < num_blocks; idx++) {
        data_to_add.emplace_back(idx, "Test " + to_string(idx));
        
        // Show progress
        if (idx > 0 && idx % progress_interval == 0) {
            cout << (idx * 100 / num_blocks) << "% ";
            cout.flush();
        }
    }
    cout << "done." << endl;

    // Initialize the ORAM client with the test data
    cout << "Initializing ORAM client (this may take a while)... ";
    cout.flush();
    Client client(data_to_add, bucket_capacity, max_range);
    cout << "done." << endl << endl;

    // Store results for each range size
    struct QueryResult {
        int range_size;
        double avg_time_per_block;
        int blocks_retrieved;
        int correct_blocks;
    };
    vector<QueryResult> results;

    // Run tests for each range size
    cout << "=== RUNNING RANGE QUERY TESTS ===" << endl;
    for (size_t test_idx = 0; test_idx < range_sizes.size(); test_idx++) {
        int range_size = range_sizes[test_idx];
        
        cout << "\nTEST " << (test_idx + 1) << ": Range size = 2^" 
             << log2(range_size) << " = " << range_size << " blocks" << endl;
        
        // Start the range query from the middle of the dataset
        // to avoid edge effects at the beginning or end
        int start_id = (num_blocks / 2) - (range_size / 2);
        if (start_id < 0) start_id = 0;
        
        cout << "Query starting at block " << start_id 
             << ", range size = " << range_size << endl;

        // Time the query
        auto query_start = chrono::high_resolution_clock::now();
        auto result = client.simple_access(start_id, range_size, 0, {});
        auto query_end = chrono::high_resolution_clock::now();
        chrono::duration<double> query_duration = query_end - query_start;

        // Count correct blocks for data integrity check
        int correct_blocks = 0;
        for (auto& blk : result) {
            int id = blk.id;
            if (id >= 0 && id < num_blocks) {
                string expected_data = "Test " + to_string(id);
                if (blk.data == expected_data) {
                    correct_blocks++;
                } else {
                    cout << "  ERROR: Block ID " << id 
                         << " has incorrect data. Expected: \"" << expected_data 
                         << "\", Got: \"" << blk.data << "\"" << endl;
                }
            }
        }

        // Compute average query access time per block
        int blocks_retrieved = result.size();
        double avg_time_per_block = query_duration.count() / 
            (blocks_retrieved > 0 ? blocks_retrieved : 1);
        
        // Store the results
        results.push_back({
            range_size,
            avg_time_per_block,
            blocks_retrieved,
            correct_blocks
        });

        // Print results for this range size
        cout << "  Retrieved " << blocks_retrieved << " blocks in " 
             << fixed << setprecision(6) << query_duration.count() << " s" << endl;
        cout << "  Correct blocks: " << correct_blocks 
             << "/" << blocks_retrieved 
             << " (" << (blocks_retrieved > 0 ? 
                         (double)correct_blocks / blocks_retrieved * 100 : 0)
             << "%)" << endl;
        cout << "  Average time per block: " 
             << fixed << setprecision(8) << avg_time_per_block << " s/block" << endl;
    }

    // Print summary table
    cout << "\n=== SUMMARY OF RESULTS ===" << endl;
    cout << "+---------------+---------------+---------------+---------------+" << endl;
    cout << "| Range Size    | Blocks        | Integrity     | Avg Time      |" << endl;
    cout << "| (power of 2)  | Retrieved     | Check         | (s/block)     |" << endl;
    cout << "+---------------+---------------+---------------+---------------+" << endl;
    
    for (const auto& res : results) {
        int power = log2(res.range_size);
        double integrity_pct = res.blocks_retrieved > 0 ? 
            (double)res.correct_blocks / res.blocks_retrieved * 100 : 0;
            
        cout << "| 2^" << setw(11) << left << power
             << "| " << setw(13) << left << res.blocks_retrieved
             << "| " << fixed << setprecision(2) << setw(11) << left << integrity_pct << "%"
             << "| " << fixed << setprecision(8) << res.avg_time_per_block << " |" << endl;
    }
    cout << "+---------------+---------------+---------------+---------------+" << endl;

    cout << "\n=== All tests completed ===" << endl;
    return 0;
}
