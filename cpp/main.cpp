#include "../include/client.h"  // Include the Client class (manages private data access).
#include "../include/server.h"  // Include the Server class (stores encrypted ORAM data).
#include "../include/bucket.h"  // Include the Bucket class (stores blocks in ORAM tree nodes).
#include "../include/bst.h"     // Include BST (possibly for organizing blocks within buckets).
#include <iostream>             // For standard input/output operations.
#include <cmath>                // For mathematical operations like log2().
#include <memory>               // For smart pointers.
#include <utility>              // For move semantics.

using namespace std;

int main() {
    // === Path ORAM Initialization ===

    // Define the total number of data blocks stored in the ORAM.
    int num_blocks = 16;

    // Define the bucket capacity, i.e., the maximum number of blocks stored in a single ORAM bucket.
    int bucket_capacity = 4;

    // Compute L, which represents the **height** of the binary ORAM tree.
    int L = ceil(log2(num_blocks));  // L = log2(num_blocks) rounded up.

    // Compute the total number of buckets in the binary ORAM tree.
    // The number of buckets in a full binary tree of height L is (2^(L+1)) - 1.
    int num_buckets = (1 << (L + 1)) - 1;
    cout << "Initializing ORAM with " << num_buckets << " buckets." << endl;

    // === Constructing ORAM Components ===

    // Initialize the ORAM tree as a **binary heap of buckets**.
    // This heap serves as the **server-side ORAM storage**.
    BucketHeap oram_tree(num_buckets, bucket_capacity);

    // Initialize the **server**, which stores the ORAM tree structure.
    // The server is responsible for handling encrypted ORAM accesses and managing storage.
    Server server(num_blocks, bucket_capacity, move(oram_tree));

    // Initialize the **client**, which interacts with the ORAM storage.
    // The client manages **position mapping**, encryption, and ORAM access patterns.
    Client client(num_blocks, &server);

    // === Writing Data to ORAM Securely ===

    // The client writes a block with **ID = 3** and stores the string "OWAM!" securely.
    // This follows the Path ORAM protocol:
    // 1. The client reads the entire **path from the block's assigned leaf to the root**.
    // 2. The client updates the block in the **stash** (local cache).
    // 3. The block is **remapped to a new random leaf** (to hide access patterns).
    // 4. The client **evicts** the updated blocks back into the ORAM tree.
    cout << "Writing block id 3." << endl;
    client.access(1, 3, "OWAM!");  // "1" indicates a write operation.

    // === Reading Data from ORAM Securely ===

    // The client **reads** the block with ID = 3.
    // Following Path ORAM:
    // 1. The client retrieves the path from the current leaf position to the root.
    // 2. It loads blocks into the stash and searches for the requested ID.
    // 3. The block is returned (if found), and the stash is **evicted back into ORAM**.
    cout << "Reading block id 3." << endl;
    block result = client.access(0, 3, "");  // "0" indicates a read operation.

    // Output the retrieved block information.
    cout << "Block id: " << result.id << ", Data: " << result.data << endl;

    return 0;
}
