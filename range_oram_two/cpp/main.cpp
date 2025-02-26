#include "../include/block.h"
#include "../include/bucket.h"
#include "../include/oram.h"
#include "../include/client.h"
#include "../include/server.h"
#include "../include/encryption.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cassert>

using namespace std;

// Utility function to print a separator line
void printSeparator(const string& title = "") {
    cout << "\n";
    cout << "============================================================" << endl;
    if (!title.empty()) {
        cout << "    " << title << endl;
        cout << "============================================================" << endl;
    }
    cout << "\n";
}

int main() {
    // Configuration parameters - start small for easier debugging
    const int NUM_BLOCKS = 16;       // Total number of blocks in the system
    const int BUCKET_CAPACITY = 4;   // Z parameter - blocks per bucket
    const int MAX_RANGE = 8;         // Maximum range size to support
    
    printSeparator("INITIALIZATION");
    cout << "Creating rORAM with:" << endl;
    cout << "- " << NUM_BLOCKS << " total blocks" << endl;
    cout << "- " << BUCKET_CAPACITY << " blocks per bucket (Z)" << endl;
    cout << "- " << MAX_RANGE << " maximum range size" << endl;
    cout << "- " << ceil(log2(MAX_RANGE)) + 1 << " sub-ORAMs" << endl;
    
    // Create a server and client
    Server server(NUM_BLOCKS, BUCKET_CAPACITY, MAX_RANGE);
    Client client(NUM_BLOCKS, BUCKET_CAPACITY, &server, MAX_RANGE);
    
    // Step 1: Initialize the system with some test data
    printSeparator("STEP 1: INITIALIZING TEST DATA");
    for (int i = 0; i < 8; i++) {
        string data = "Block " + to_string(i) + " data";
        cout << "Writing block " << i << " with data: '" << data << "'" << endl;
        client.access(i, 1, 1, data);
    }
    
    // Check state after initialization
    printSeparator("STATE AFTER INITIALIZATION");
    client.print_position_maps();
    client.print_stashes();
    client.print_tree_state(0); // Show tree R0 state
    
    // Step 2: Test simple read operation
    printSeparator("STEP 2: SIMPLE READ TEST");
    int test_block_id = 3;
    cout << "Reading block " << test_block_id << endl;
    string read_result = client.access(test_block_id, 1, 0, "");
    cout << "Read result: '" << read_result << "'" << endl;
    
    // Check state after read
    printSeparator("STATE AFTER READ");
    client.print_stashes();
    
    // Step 3: Test simple write operation
    printSeparator("STEP 3: SIMPLE WRITE TEST");
    int write_block_id = 5;
    string write_data = "Updated block 5";
    cout << "Writing block " << write_block_id << " with data: '" << write_data << "'" << endl;
    client.access(write_block_id, 1, 1, write_data);
    
    // Verify the write
    cout << "Reading back block " << write_block_id << " to verify write" << endl;
    string verify_data = client.access(write_block_id, 1, 0, "");
    cout << "Read result: '" << verify_data << "'" << endl;
    if (verify_data == write_data) {
        cout << "PASS: Write operation successful" << endl;
    } else {
        cout << "FAIL: Write verification failed" << endl;
        cout << "  Expected: '" << write_data << "'" << endl;
        cout << "  Got: '" << verify_data << "'" << endl;
    }
    
    // Step 4: Test range write
    printSeparator("STEP 4: RANGE WRITE TEST");
    int range_start = 4;
    int range_size = 4;
    string range_data = "Range data";
    
    cout << "Writing '" << range_data << "' to range [" << range_start 
         << ", " << (range_start + range_size - 1) << "]" << endl;
    
    client.access(range_start, range_size, 1, range_data);
    
    // Verify range write
    cout << "Verifying range write:" << endl;
    for (int i = range_start; i < range_start + range_size; i++) {
        string block_data = client.access(i, 1, 0, "");
        cout << "Block " << i << " data: '" << block_data << "'" << endl;
        if (block_data == range_data) {
            cout << "PASS: Block " << i << " contains expected data" << endl;
        } else {
            cout << "FAIL: Block " << i << " verification failed" << endl;
            cout << "  Expected: '" << range_data << "'" << endl;
            cout << "  Got: '" << block_data << "'" << endl;
        }
    }
    
    // Detailed check of all trees to verify data consistency
    printSeparator("VERIFYING DATA CONSISTENCY ACROSS ALL TREES");
    for (int tree_idx = 0; tree_idx <= ceil(log2(MAX_RANGE)); tree_idx++) {
        cout << "=== TREE R" << tree_idx << " DATA CHECK ===" << endl;
        
        // For each block in the range
        for (int blk_id = range_start; blk_id < range_start + range_size; blk_id++) {
            // Look for this block in the tree's bucket levels
            bool found = false;
            ORAM* tree = client.oram_trees[tree_idx];
            int h = ceil(log2(tree->num_buckets));
            
            // Check each level of the tree
            for (int level = 0; level < h; level++) {
                int level_start = (1 << level) - 1;
                int level_end = (1 << (level+1)) - 1;
                
                for (int bucket_idx = level_start; bucket_idx < level_end && bucket_idx < tree->num_buckets; bucket_idx++) {
                    int normal_idx = tree->toNormalIndex(bucket_idx);
                    Bucket bucket = tree->read_bucket(normal_idx);
                    
                    // Check each block in the bucket
                    for (const block& b : bucket.getBlocks()) {
                        if (!b.dummy && b.id == blk_id) {
                            cout << "Block " << blk_id << " found in tree R" << tree_idx 
                                 << ", level " << level << ", bucket " << bucket_idx 
                                 << " with data: '" << b.data << "'" << endl;
                            found = true;
                            if (b.data == range_data) {
                                cout << "  PASS: Data matches expected value" << endl;
                            } else {
                                cout << "  FAIL: Data doesn't match expected value" << endl;
                                cout << "    Expected: '" << range_data << "'" << endl;
                                cout << "    Got: '" << b.data << "'" << endl;
                            }
                        }
                    }
                }
            }
            
            if (!found) {
                cout << "Block " << blk_id << " not found in tree R" << tree_idx << endl;
            }
        }
    }
    
    // Final check on all trees
    printSeparator("FINAL SYSTEM STATE");
    for (int i = 0; i <= ceil(log2(MAX_RANGE)); i++) {
        client.print_tree_state(i);
    }
    client.print_position_maps();
    client.print_stashes();
    
    printSeparator("DEBUG COMPLETE");
    return 0;
}