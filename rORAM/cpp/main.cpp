using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "../include/client.h"
#include "../include/server.h"
#include "../include/oram.h"

using namespace std;

int main() {
    int num_buckets = 31; // must be 2^i-1 where this is closest power of 2 above to the number of blocks you want to support
    int bucket_capacity = 4;
    int max_range = 17;  //must be 2^i+1 where this is closest power of 2 above to the largest range you want 

    cout << "=== ORAM ALGORITHM TESTING ===" << endl;

    cout << "Initializing client with at least" << num_buckets << " buckets" << " of size "
         << bucket_capacity << ", max range " << max_range << endl;
    
    //make data
    vector<pair<int,string>> data_to_add;
    for (int i = 0; i < num_buckets; i++) {
        string data = "Test " + to_string(i);
        data_to_add.push_back(make_pair(i,data));
    }

    Client client(data_to_add, bucket_capacity, max_range);

///*
    for (int i = 0; i < client.num_trees; i++){
        client.printLogicalTreeState(i,10);
    }
    //client.print_position_maps();
///*
    cout << "Reading blocks from oram tree" << endl;
///*
    auto result1 = client.simple_access(1, 4, 0, {});
    auto result2 = client.simple_access(12, 15, 0, {});
    //auto result2 = client.simple_access(1022, 8, 0, {});
    
///*
    cout << "Read result 1:" << endl;
    for (const auto& blk : result1) {
        cout << "  Block ID: " << blk.id << ", Data: '" << blk.data << "'" << endl;
    }
///*
    cout << "read result 2:" << endl;
    for (const auto& blk : result2) {
        cout << "  Block ID: " << blk.id << ", Data: '" << blk.data << "'" << endl;
    }
///*

    cout << "\n=== All tests completed ===" << endl;
    
    return 0;
}