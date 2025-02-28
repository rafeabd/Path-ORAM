using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "../include/client.h"
#include "../include/server.h"
#include "../include/oram.h"

using namespace std;

// Test the read range 
void test_read_range(Client& client) {
    cout << "\n===== TESTING ALGORITHM 1: READ_RANGE =====" << endl;
    client.init_test_data();
    cout << "Initial stash state:" << endl;
    client.print_stashes();
    int range_power = 1; 
    int id = 0;
    
    cout << "\nExecuting read_range(range_power=" << range_power << ", id=" << id << ")" << endl;
    auto [blocks, p_prime] = client.read_range(range_power, id);
    
    cout << "read_range returned " << blocks.size() << " blocks and p_prime=" << p_prime << endl;
    cout << "Returned blocks:" << endl;
    for (const auto& blk : blocks) {
        cout << "  Block ID: " << blk.id << ", Data: '" << blk.data << "'" << endl;
    }
    
    range_power = 2; 
    id = 4;
    
    cout << "\nExecuting read_range(range_power=" << range_power << ", id=" << id << ")" << endl;
    auto [blocks2, p_prime2] = client.read_range(range_power, id);
    
    cout << "read_range returned " << blocks2.size() << " blocks and p_prime=" << p_prime2 << endl;
    cout << "Returned blocks:" << endl;
    for (const auto& blk : blocks2) {
        cout << "  Block ID: " << blk.id << ", Data: '" << blk.data << "'" << endl;
    }

    cout << "\nStash state after read_range tests:" << endl;
    client.print_stashes();
    
    cout << "\nPosition map state after read_range tests:" << endl;
    client.print_position_maps();
}

// Test batch evict
void test_batch_evict(Client& client) {
    cout << "\n===== TESTING ALGORITHM 2: BATCH_EVICT =====" << endl;
    client.init_test_data();
    cout << "Initial stash state:" << endl;
    client.print_stashes();
    
    cout << "\nReading blocks into stash with read_range..." << endl;
    client.read_range(2, 0); 
    
    cout << "\nStash state before batch_evict:" << endl;
    client.print_stashes();
    
    int eviction_number = 2; 
    int range_power = 2; 
    
    cout << "\nExecuting batch_evict(eviction_number=" << eviction_number 
         << ", range_power=" << range_power << ")" << endl;
    client.batch_evict(eviction_number, range_power);
    
    cout << "\nStash state after batch_evict:" << endl;
    client.print_stashes();
    
    cout << "\nTree state after batch_evict:" << endl;
    client.print_tree_state(range_power, 2); 
}

// Test access
void test_access(Client& client) {
    cout << "\n===== TESTING ALGORITHM 3: ACCESS =====" << endl;
    for (int i = 0; i < 50; i++) {
        string data = "Test " + to_string(i);
        cout << "\nWriting block " << i << " with data '" << data << "'" << endl;
        string result = client.access(i, 1, 1, data);
    }
    
    cout << "\nStash state after initialization:" << endl;
    client.print_stashes();

    for (int i = 0; i < 50; i++) {
        cout << "\nReading block " << i << endl;
        string result = client.access(i, 1, 0, "");
        cout << "Read returned: '" << result << "'" << endl;
        string expected = "Test " + to_string(i);
        cout << (result == expected ? "SUCCESS" : "ERROR") << endl;
    }
    
    cout << "\nWriting to range [2, 5] with data 'Range data'" << endl;
    client.access(2, 4, 1, "Range data");
    
    cout << "\nReading range [2, 5]" << endl;
    string range_result = client.access(2, 4, 0, "");
    cout << "Range read returned: '" << range_result << "'" << endl;
    
    for (int i = 2; i <= 5; i++) {
        cout << "\nVerifying block " << i << endl;
        string result = client.access(i, 1, 0, "");
        cout << "Read returned: '" << result << "'" << endl;
        cout << (result == "Range data" ? "SUCCESS" : "ERROR") << endl;
    }
}

int main() {
    int num_buckets = 100;
    int bucket_capacity = 4;
    int max_range = 50;
    

    Server server(num_buckets, bucket_capacity, max_range);
    Client client(num_buckets, bucket_capacity, &server, max_range);
    
    cout << "=== ORAM ALGORITHM TESTING ===" << endl;
    cout << "Initialized client with " << num_buckets << " buckets" << " of size "
         << bucket_capacity << ", max range " << max_range << endl;

    for (int i = 0; i < 100; i++) {
        string data = "Test " + to_string(i);
        string result = client.access(i, 1, 1, data);
    }
    
    
    //client.print_stashes();
    for(int i =0; i< client.oram_trees.size(); i++){
        //client.print_tree_state(i,3);
    }
    //client.print_stashes();
    //client.print_position_maps();

    cout << "reading data" << endl;
    cout << "read data:" << client.access(0,50,0,"") << endl;


    /*
    test_read_range(client);
    
    // Reinitialize client for clean state
    Client client2(num_buckets, bucket_capacity, &server, max_range);
    test_batch_evict(client2);
    
    // Reinitialize client for clean state
    Client client3(num_buckets, bucket_capacity, &server, max_range);
    test_access(client3);
    */
   
    //cout << client.access(1,30,0,"") << endl;

    //cout << "reading" << endl;
    //cout << client.access(1,5,0,"") << endl;
    //cout << client.access(25,25,0,"") << endl;
    //cout << client.access(50,25,0,"") << endl;
    //cout << client.access(75,25,0,"") << endl;
    /*
    for (int i = 0; i < 5; i++) {
        string result = client.access(i, 1, 0, "");
        string expected = "Test " + to_string(i);
        cout << (result == expected ? "SUCCESS" : "ERROR") << endl;
    }
    */
    cout << "\n=== All tests completed ===" << endl;
    
    return 0;
}