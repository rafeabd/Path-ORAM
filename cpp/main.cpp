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
    try {
        std::cout << "Initializing Path ORAM with BucketHeap storage...\n";
        Client client(8);  // Initialize with space for 8 blocks (3 levels)

        std::cout << "\nInitial tree state:\n";
        client.getTree()->printHeap();

        // Test 1: Write some blocks
        std::cout << "\n=== Writing blocks ===\n";
        client.access(1, 1, "First block");
        client.access(1, 2, "Second block");
        client.access(1, 3, "Third block");

        std::cout << "\nTree state after writes:\n";
        client.getTree()->printHeap();

        // Test 2: Read a block
        std::cout << "\n=== Reading block 2 ===\n";
        block read_result = client.access(0, 2);
        std::cout << "Read result - ID: " << read_result.id 
                  << ", Data: " << read_result.data << "\n";

        std::cout << "\nTree state after read:\n";
        client.getTree()->printHeap();

        // Test 3: Update a block
        std::cout << "\n=== Updating block 1 ===\n";
        client.access(1, 1, "Updated first block");

        std::cout << "\nFinal tree state:\n";
        client.getTree()->printHeap();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}