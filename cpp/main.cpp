#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include "../include/client.h"
#include "../include/encryption.h"
#include <iostream>
#include <stdexcept>
#include <cassert>

using namespace std;

int main() {
    cout << "Initializing Path ORAM with BucketHeap storage...\n";
    Client client(10000);  // for x blocks - not buckets

    /*
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    cout << client.getRandomLeaf() << endl;
    */
    

    for (int i = 1; i <= 10000; i++) {
        client.access(1, i, "Block " + to_string(i));
    }

    //vector<block> path = client.range_query(1, 999);

    //block b = client.access(0, 900);
    //b.print_block();

    //for (int i = 0; i < 3; i++) {
    //    path[i].print_block();
    //}

return 0;
}