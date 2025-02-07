#include "../include/bst.h"
#include "../include/block.h"
#include "../include/bucket.h"
#include "../include/client.h"
#include "../include/encryption.h"
#include <iostream>
#include <stdexcept>

using namespace std;

int main() {
    std::cout << "Creating binary heap with 7 buckets (capacity 3 each)...\n\n";
    BucketHeap heap(7, 3);  // Creates a complete binary tree of height 2

    // Create some test blocks
    block b1(1, 10, "block1", false);
    block b2(2, 20, "block2", false);
    block b3(3, 30, "block3", false);
    block b4(4, 40, "block4", false);
    block b5(5, 50, "block5", false);

    std::cout << "Adding blocks to various buckets...\n";
    // Add blocks to create an interesting pattern
    heap.addBlockToBucket(0, b1);  // root
    heap.addBlockToBucket(1, b2);  // left child of root
    heap.addBlockToBucket(2, b3);  // right child of root
    heap.addBlockToBucket(3, b4);  // left child of node 1
    heap.addBlockToBucket(4, b5);  // right child of node 1

    std::cout << "\nPrinting heap structure:\n";
    std::cout << "------------------------\n";
    heap.printHeap();

    std::cout << "\nTesting bucket access...\n";
    std::cout << "Accessing bucket 2 blocks:\n";
    Bucket& bucket2 = heap.getBucket(2);
    for (const auto& block : bucket2.getBlocks()) {
        if (block.id != 0) {  // Assuming 0 is default/empty ID
            std::cout << "Found block with ID: " << block.id << "\n";
        }
    }

    // No need to test add/remove on potentially uninitialized buckets
    std::cout << "\nHeap structure remains unchanged:\n";
    std::cout << "--------------------\n";
    heap.printHeap();

    return 0;
}