/**
 * @class Bucket
 * @brief Container for blocks in the ORAM tree
 * 
 * Each bucket has a fixed capacity (Z) and can hold both real and dummy blocks.
 * For rORAM extension, consider adding:
 * - Level information for hierarchical storage
 * - Access frequency tracking
 * - Hot/cold block separation
 */
class Bucket {
private:
    vector<block> blocks;  // Stored blocks
    int Z;                // Maximum bucket capacity

    // TODO for rORAM:
    // int level;         // Storage level in hierarchy
    // vector<block> hot_blocks;   // Frequently accessed blocks
    // vector<block> cold_blocks;  // Rarely accessed blocks
    // timestamp_t last_access;    // Last bucket access time

public:
    /**
     * @brief Constructs a bucket with specified capacity
     * @param capacity Maximum number of blocks the bucket can hold
     */
    explicit Bucket(int capacity = 4);

    /**
     * @brief Adds a block to the bucket
     * @param block Block to be added
     * @return true if addition successful, false if bucket full
     */
    bool addBlock(const block& block);

    /**
     * @brief Initial block addition during setup
     * @param newBlock Block to be added
     * @return true if addition successful, false if bucket full
     */
    bool startaddblock(block& newBlock);

    /**
     * @brief Removes and returns all blocks from the bucket
     * @return Vector of removed blocks
     */
    vector<block> removeAllBlocks();

    /**
     * @brief Removes a specific block by ID
     * @param id ID of block to remove
     * @return Removed block (dummy block if not found)
     */
    block remove_block(int);

    /**
     * @brief Gets reference to stored blocks
     * @return Reference to vector of blocks
     */
    vector<block>& getBlocks();

    /**
     * @brief Checks if bucket has space for more blocks
     * @return true if bucket can store more blocks
     */
    bool hasSpace();

    /**
     * @brief Gets current number of blocks
     * @return Number of blocks in bucket
     */
    size_t size() const { return blocks.size(); }

    /**
     * @brief Gets maximum bucket capacity
     * @return Bucket capacity (Z)
     */
    int capacity() const { return Z; }

    /**
     * @brief Clears all blocks from bucket
     */
    void clear();

    /**
     * @brief Prints bucket contents for debugging
     */
    void print_bucket();

    // TODO for rORAM - Additional methods:
    // void promoteHotBlocks();    // Move frequently accessed blocks to hot storage
    // void demoteColdBlocks();    // Move rarely accessed blocks to cold storage
    // bool isHot() const;         // Check if bucket contains mostly hot blocks
    // void updateAccessTime();    // Update last access timestamp
    // void compressBucket();      // Compress bucket contents for storage efficiency
};

#endif
