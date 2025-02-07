#ifndef CLIENT_H
#define CLIENT_H

#include "block.h"
#include "bucket.h"
#include "bst.h"
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

class Client {
private:
    std::vector<unsigned char> key_for_id;
    std::vector<unsigned char> key_for_data;
    std::vector<block> stash;
    std::map<int, int> position_map;
    int L;
    std::shared_ptr<BucketHeap> tree;  // Storage structure

    // Generates a random leaf number in the range [0, (1 << L) - 1]
    int getRandomLeaf();
    
    // Determines if a given bucket (by tree index) is on the path
    // from a block's assigned leaf to the root.
    bool isOnPath(int, int);
    
    // Helper methods for tree operations:
    // Returns the list of tree indices from the leaf node to the root.
    std::vector<int> getPath(int leaf);
    
    // Reads all blocks stored along the path from the given leaf to the root.
    std::vector<block> readPath(int leaf);
    
    // Writes blocks from the stash back to the tree along the path from the given leaf.
    // Blocks that are successfully placed are removed from the stash.
    void writePath(int leaf, std::vector<block>& stash);

    void evict(const std::vector<block>& );

public:
    // Constructs a Client with a given number of blocks (used to determine tree height).
    Client(int num_blocks);

    // Access method: op == 0 for read, op == 1 for write.
    // For read operations, the 'data' parameter is ignored.
    block access(int op, int id, const std::string& data = "");
    
    // Getter for testing or debugging purposes.
    BucketHeap* getTree();
};

#endif // CLIENT_H
