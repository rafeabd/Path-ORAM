/**
 * @file client.h
 * @brief Defines the client interface for Path ORAM operations.
 * 
 * The Client class manages client-side Path ORAM operations including:
 * - Position map maintenance
 * - Stash management
 * - Block access operations
 * - Path retrieval and writing
 * 
 * For rORAM extension:
 * 1. Implement hierarchical position map
 * 2. Add write buffer management
 * 3. Support background operations
 * 4. Track access patterns
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include "bucket.h"
#include "bst.h"
#include "server.h"
#include "encryption.h"
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;

/**
 * @class Client
 * @brief Manages client-side Path ORAM operations
 * 
 * For rORAM extension:
 * - Add hierarchical storage management
 * - Implement write buffering
 * - Support background eviction
 * - Track block access patterns
 */
class Client {
private:
    vector<unsigned char> key;                // Encryption key
    unordered_map<int, block> stash;         // Temporary block storage
    map<int, int> position_map;              // Maps block IDs to leaf positions
    int L;                                   // Tree height
    Server* server;                          // Server reference
    
    /**
     * @brief Checks if block's leaf is on path to bucket
     * @param blockLeaf Leaf position of block
     * @param bucketIndex Target bucket index
     * @return true if block's path includes bucket
     */
    bool isOnPath(int blockLeaf, int bucketIndex);

    /**
     * @brief Reads path from server
     * @param leaf Target leaf position
     * @return Vector of buckets in path
     * 
     * For rORAM:
     * - Add hierarchical path reading
     * - Implement prefetching
     * - Track access patterns
     */
    vector<Bucket> readPath(int leaf);

    /**
     * @brief Writes path back to server
     * @param leaf Target leaf position
     * @param path_buckets Buckets to write
     * 
     * For rORAM:
     * - Add write buffering
     * - Implement background writes
     * - Support partial path updates
     */
    void writePath(int leaf, vector<Bucket>& path_buckets);
    
public:
    /**
     * @brief Gets path from leaf to root
     * @param leaf Starting leaf position
     * @return Vector of bucket indices in path
     */
    vector<int> getPath(int leaf);

    /**
     * @brief Generates random leaf position
     * @return Random leaf index
     */
    int getRandomLeaf();

    /**
     * @brief Initializes client with given parameters
     * @param num_blocks Number of blocks in ORAM
     * @param server_ptr Pointer to server
     * @param encryptionKey Encryption key
     */
    Client(int num_blocks, Server* server_ptr, const vector<unsigned char>& encryptionKey);

    /**
     * @brief Performs block access operation
     * @param op Operation type (0=read, 1=write)
     * @param id Block ID
     * @param data Data for write operation
     * @return Accessed block
     * 
     * For rORAM:
     * - Add access pattern optimization
     * - Implement write buffering
     * - Support batch operations
     */
    block access(int op, int id, const string& data = "");

    /**
     * @brief Performs range query
     * @param start Start block ID
     * @param end End block ID
     * @return Vector of blocks in range
     * 
     * For rORAM:
     * - Optimize for range queries
     * - Add prefetching
     * - Implement batch access
     */
    vector<block> range_query(int start, int end);

    /**
     * @brief Prints stash contents for debugging
     */
    void print_stash();
};

/* TODO for rORAM - Additional components to consider:
 * 
 * class WriteBuffer {
 *     queue<WriteOperation> pending_writes;
 *     void process_writes();
 *     void merge_operations();
 * };
 * 
 * class AccessTracker {
 *     map<int, int> access_counts;
 *     vector<int> hot_blocks;
 *     void update_patterns();
 * };
 * 
 * class BackgroundProcessor {
 *     thread eviction_thread;
 *     void perform_eviction();
 *     void optimize_placement();
 * };
 */

#endif
