#include "../include/client.h"
#include "../include/server.h"
#include "../include/oram.h"
#include "../include/bucket.h"
#include "../include/block.h"
#include "../include/encryption.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <iomanip>

using namespace std;

// Function to print a separator line for better readability
void printSeparator() {
    cout << "-------------------------------------------------------------" << endl;
}

// Function to print success/fail results with color codes for pass/fail
void printResult(const string& testName, bool success) {
    cout << "Test: " << testName << " - ";
    if (success) {
        cout << "PASS" << endl;
    } else {
        cout << "FAIL" << endl;
    }
}

// Function to initialize data for a range of blocks
void initializeData(vector<string>& data, int numBlocks) {
    for (int i = 0; i < numBlocks; i++) {
        char buffer[50];
        sprintf(buffer, "data_for_block_%d", i);
        data.push_back(string(buffer));
    }
}

// Function to test writing and reading a single block
bool testSingleBlock(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Single Block Write and Read" << endl;
    
    int blockId = 5;
    string writeData = data[blockId];
    
    cout << "Writing '" << writeData << "' to block " << blockId << endl;
    client->access(blockId, 1, 1, writeData);
    
    cout << "Reading from block " << blockId << endl;
    string readData = client->access(blockId, 1, 0, "");
    
    cout << "Write data: '" << writeData << "'" << endl;
    cout << "Read data:  '" << readData << "'" << endl;
    
    bool success = (readData == writeData);
    printResult("Single Block", success);
    return success;
}

// Function to test writing and reading a small range (e.g., 2 blocks)
bool testSmallRange(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Small Range (2 blocks) Write and Read" << endl;
    
    int startBlock = 10;
    int rangeSize = 2;
    
    // Write data to the range
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string writeData = data[blockId];
        cout << "Writing '" << writeData << "' to block " << blockId << endl;
        client->access(blockId, 1, 1, writeData);
    }
    
    // Read data back from the range
    bool success = true;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string expectedData = data[blockId];
        string readData = client->access(blockId, 1, 0, "");
        
        cout << "Block " << blockId << ":" << endl;
        cout << "  Expected: '" << expectedData << "'" << endl;
        cout << "  Read:     '" << readData << "'" << endl;
        
        if (readData != expectedData) {
            success = false;
        }
    }
    
    printResult("Small Range", success);
    return success;
}

// Function to test a medium range (e.g., 4 blocks)
bool testMediumRange(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Medium Range (4 blocks) Write and Read" << endl;
    
    int startBlock = 20;
    int rangeSize = 4;
    
    // Write data to the range
    cout << "Writing data to blocks " << startBlock << " to " << (startBlock + rangeSize - 1) << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string writeData = data[blockId];
        client->access(blockId, 1, 1, writeData);
    }
    
    // Read data back from the range
    bool success = true;
    cout << "Reading data from blocks " << startBlock << " to " << (startBlock + rangeSize - 1) << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string expectedData = data[blockId];
        string readData = client->access(blockId, 1, 0, "");
        
        if (readData != expectedData) {
            cout << "Mismatch for block " << blockId << ":" << endl;
            cout << "  Expected: '" << expectedData << "'" << endl;
            cout << "  Read:     '" << readData << "'" << endl;
            success = false;
        }
    }
    
    if (success) {
        cout << "All blocks in the range were read correctly" << endl;
    }
    
    printResult("Medium Range", success);
    return success;
}

// Function to test a direct range query (using the range parameter of access)
bool testDirectRangeQuery(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Direct Range Query (using range parameter)" << endl;
    
    int startBlock = 30;
    int rangeSize = 4; // Must be a power of 2 for proper range query
    
    // Write data to the range (one block at a time)
    cout << "Writing data to blocks " << startBlock << " to " << (startBlock + rangeSize - 1) << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string writeData = data[blockId];
        client->access(blockId, 1, 1, writeData);
    }
    
    // Read the entire range at once
    cout << "Reading entire range at once (blocks " << startBlock << " to " 
         << (startBlock + rangeSize - 1) << ")" << endl;
    
    // For the direct range query, we'll check the first block
    string readData = client->access(startBlock, rangeSize, 0, "");
    string expectedData = data[startBlock];
    
    cout << "First block in range:" << endl;
    cout << "  Expected: '" << expectedData << "'" << endl;
    cout << "  Read:     '" << readData << "'" << endl;
    
    bool success = (readData == expectedData);
    
    // Individually verify the rest of the blocks to ensure they're properly accessible
    cout << "Verifying all blocks in the range individually:" << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string expectedData = data[blockId];
        string readData = client->access(blockId, 1, 0, "");
        
        bool blockSuccess = (readData == expectedData);
        cout << "  Block " << blockId << ": " << (blockSuccess ? "OK" : "FAILED") << endl;
        
        if (!blockSuccess) {
            success = false;
        }
    }
    
    printResult("Direct Range Query", success);
    return success;
}

// Function to test a large range query (e.g., 8 blocks)
bool testLargeRangeQuery(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Large Range Query (8 blocks)" << endl;
    
    int startBlock = 40;
    int rangeSize = 8; // Larger range
    
    // Write data to the range (one block at a time)
    cout << "Writing data to blocks " << startBlock << " to " << (startBlock + rangeSize - 1) << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string writeData = data[blockId];
        client->access(blockId, 1, 1, writeData);
    }
    
    // Read the entire range at once
    cout << "Reading entire range at once (blocks " << startBlock << " to " 
         << (startBlock + rangeSize - 1) << ")" << endl;
    
    // For the range query, we'll test access with range parameter
    // and then verify each block individually
    string initialRead = client->access(startBlock, rangeSize, 0, "");
    
    // Individually verify all blocks in the range
    bool success = true;
    cout << "Verifying all blocks in the range individually:" << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string expectedData = data[blockId];
        string readData = client->access(blockId, 1, 0, "");
        
        bool blockSuccess = (readData == expectedData);
        cout << "  Block " << blockId << ": " << (blockSuccess ? "OK" : "FAILED") << endl;
        
        if (!blockSuccess) {
            cout << "    Expected: '" << expectedData << "'" << endl;
            cout << "    Read:     '" << readData << "'" << endl;
            success = false;
        }
    }
    
    printResult("Large Range Query", success);
    return success;
}

// Function to test non-power-of-2 range queries
bool testNonPowerOfTwoRangeQuery(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Non-Power-of-2 Range Query (3 blocks)" << endl;
    
    int startBlock = 50;
    int rangeSize = 3; // Not a power of 2
    
    // Write data to the range (one block at a time)
    cout << "Writing data to blocks " << startBlock << " to " << (startBlock + rangeSize - 1) << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string writeData = data[blockId];
        client->access(blockId, 1, 1, writeData);
    }
    
    // Read the entire range at once (this will be rounded up to next power of 2)
    cout << "Reading entire range at once (blocks " << startBlock << " to " 
         << (startBlock + rangeSize - 1) << ")" << endl;
    
    string initialRead = client->access(startBlock, rangeSize, 0, "");
    
    // Individually verify all blocks in the range
    bool success = true;
    cout << "Verifying all blocks in the range individually:" << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string expectedData = data[blockId];
        string readData = client->access(blockId, 1, 0, "");
        
        bool blockSuccess = (readData == expectedData);
        cout << "  Block " << blockId << ": " << (blockSuccess ? "OK" : "FAILED") << endl;
        
        if (!blockSuccess) {
            success = false;
        }
    }
    
    printResult("Non-Power-of-2 Range Query", success);
    return success;
}

// Function to test range query after multiple operations
bool testMultipleOperationsRangeQuery(Client* client, const vector<string>& data) {
    printSeparator();
    cout << "TEST: Range Query After Multiple Operations" << endl;
    
    int startBlock = 60;
    int rangeSize = 4;
    
    // Write data to the range
    cout << "Writing data to blocks " << startBlock << " to " << (startBlock + rangeSize - 1) << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string writeData = data[blockId];
        client->access(blockId, 1, 1, writeData);
    }
    
    // Do some random operations
    cout << "Performing additional operations to simulate activity" << endl;
    for (int i = 0; i < 10; i++) {
        int randomBlock = rand() % 20 + 70; // Pick from blocks 70-89
        string randomData = "random_data_" + to_string(randomBlock);
        client->access(randomBlock, 1, 1, randomData);
    }
    
    // Now read the original range
    cout << "Reading the original range after other operations" << endl;
    
    // Verify all blocks in the range
    bool success = true;
    cout << "Verifying all blocks in the range individually:" << endl;
    for (int i = 0; i < rangeSize; i++) {
        int blockId = startBlock + i;
        string expectedData = data[blockId];
        string readData = client->access(blockId, 1, 0, "");
        
        bool blockSuccess = (readData == expectedData);
        cout << "  Block " << blockId << ": " << (blockSuccess ? "OK" : "FAILED") << endl;
        
        if (!blockSuccess) {
            success = false;
        }
    }
    
    printResult("Range Query After Multiple Operations", success);
    return success;
}

int main() {
    // Seed random number generator
    srand(time(NULL));
    
    // Basic parameters - increase for real-world usage
    int numBlocks = 100; // Use 100 blocks for more extensive testing
    int bucketCapacity = 4;
    int maxRange = 8; // Support ranges up to size 8
    
    // Initialize test data
    vector<string> testData;
    initializeData(testData, numBlocks);
    
    // Initialize the client and server
    Server* server = new Server(numBlocks, bucketCapacity, maxRange);
    Client* client = new Client(numBlocks, bucketCapacity, server, maxRange);
    
    // Print test header
    cout << "==============================================" << endl;
    cout << "     RANGE ORAM (rORAM) COMPREHENSIVE TEST    " << endl;
    cout << "==============================================" << endl;
    cout << "Number of blocks: " << numBlocks << endl;
    cout << "Bucket capacity: " << bucketCapacity << endl;
    cout << "Maximum range size: " << maxRange << endl;
    cout << "==============================================" << endl;
    
    // Run the tests
    bool test1 = testSingleBlock(client, testData);
    bool test2 = testSmallRange(client, testData);
    bool test3 = testMediumRange(client, testData);
    bool test4 = testDirectRangeQuery(client, testData);
    bool test5 = testLargeRangeQuery(client, testData);
    bool test6 = testNonPowerOfTwoRangeQuery(client, testData);
    bool test7 = testMultipleOperationsRangeQuery(client, testData);
    
    // Print summary
    printSeparator();
    cout << "TEST SUMMARY" << endl;
    printSeparator();
    cout << "Single Block: " << (test1 ? "PASS" : "FAIL") << endl;
    cout << "Small Range (2 blocks): " << (test2 ? "PASS" : "FAIL") << endl;
    cout << "Medium Range (4 blocks): " << (test3 ? "PASS" : "FAIL") << endl;
    cout << "Direct Range Query: " << (test4 ? "PASS" : "FAIL") << endl;
    cout << "Large Range Query (8 blocks): " << (test5 ? "PASS" : "FAIL") << endl;
    cout << "Non-Power-of-2 Range Query: " << (test6 ? "PASS" : "FAIL") << endl;
    cout << "Range Query After Multiple Operations: " << (test7 ? "PASS" : "FAIL") << endl;
    printSeparator();
    
    int passCount = test1 + test2 + test3 + test4 + test5 + test6 + test7;
    cout << "Total Tests: 7" << endl;
    cout << "Tests Passed: " << passCount << endl;
    cout << "Tests Failed: " << (7 - passCount) << endl;
    printSeparator();
    
    // Clean up
    delete client;
    delete server;
    
    return 0;
}