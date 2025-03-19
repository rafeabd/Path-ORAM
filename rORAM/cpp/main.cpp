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

int main(int argc, char* argv[]) {
    const int dataset_size_power = 10;
    const int num_blocks = 1 << dataset_size_power; 
    
    const vector<int> range_sizes = {

        1 << 1,  
        1 << 2,
        1 << 3,
        1 << 4,  
        //1 << 5,
        1 << 6,
        //1 << 7,
        1 << 8,
        //1 << 9,
        1 << 10, 
        //1 << 11,
        //1 << 12,
        //1 << 13,
        //1 << 14,  
        //1 << 15

    };

    int power = 1;
    while ((1 << power) - 1 < num_blocks) {
        power++;
    }
    int num_buckets = (1 << power) - 1;
    int bucket_capacity = 4;
    
    int max_range_power = 10;
    int max_range = (1 << (max_range_power + 1)) + 1; 

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
    
    cout << "Creating test dataset... ";
    vector<pair<int, string>> data_to_add;
    data_to_add.reserve(num_blocks);
    
    for (int idx = 0; idx < num_blocks; idx++) {
        data_to_add.emplace_back(idx, "Test " + to_string(idx));
        
    }
    cout << "done." << endl;

    cout << "Initializing ORAM client (this may take a while)... ";
    cout.flush();
    Client client(data_to_add, bucket_capacity, max_range);
    cout << "done." << endl << endl;

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
        
        int start_id = (num_blocks / 2) - (range_size / 2);
        if (start_id < 0) start_id = 0;
        
        cout << "Query starting at block " << start_id 
             << ", range size = " << range_size << endl;

        // Time the query
        auto query_start = chrono::high_resolution_clock::now();
        auto result = client.simple_access(start_id, range_size, 0, {});
        auto query_end = chrono::high_resolution_clock::now();
        chrono::duration<double> query_duration = query_end - query_start;

        // Check correct blocks for data integrity check
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
