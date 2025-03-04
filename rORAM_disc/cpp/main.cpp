using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "../include/client.h"
#include "../include/server.h"
#include "../include/oram.h"
#include "../include/encryption.h"

using namespace std;

int main() {

    /*
    vector<unsigned char> key = generateEncryptionKey(64);
    Bucket test_bucket = Bucket();
    for (int i = 0; i<3;i++){
        block block_to_add = block(i,to_string(i),false,vector<int> {});
        test_bucket.addBlock(encryptBlock(block_to_add,key));
    }
    block add_dummy = block();
    test_bucket.addBlock(encryptBlock(add_dummy, key));
    string serialized_bucket = serialize_bucket(test_bucket);
    */


   vector<unsigned char> key = generateEncryptionKey(64);
   ORAM test_tree = ORAM(32, 4, key, 3, "test_tree");
/*
///*
    for (int i = 0; i < client.num_trees; i++){
        client.printLogicalTreeState(i,10);
    }
    //client.print_position_maps();
///*
    cout << "Reading blocks from oram tree" << endl;
////*
    auto result1 = client.simple_access(1, 4, 0, {});
    auto result2 = client.simple_access(90, 4, 0, {});
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

    cout << "\n=== All tests completed ===" << endl;
    
*/
    return 0;
}