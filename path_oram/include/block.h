/**
 * @file block.h
 * @brief Defines the fundamental block structure for Path ORAM.
 * 
 * This file contains the block structure which represents the basic unit of data
 * in the Path ORAM system. Each block contains metadata (leaf, id, dummy status)
 * and the actual data content.
 * 
 * Extension notes for rORAM:
 * - Consider adding timestamp field for recency tracking
 * - Add size tracking for variable-sized blocks
 * - Include access frequency counter
 */

#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"
#include <string>

using namespace std;

/**
 * @struct block
 * @brief Basic data unit in the ORAM system.
 * 
 * Each block represents a single data unit that can be stored in the ORAM tree.
 * Blocks can be either real data blocks or dummy blocks used for padding.
 */
struct block {
    int leaf;       // Target leaf node in the ORAM tree
    int id;         // Unique identifier for the block
    string data;    // Actual data content
    bool dummy;     // Flag indicating if this is a dummy block

    /**
     * @brief Constructs a new block
     * @param id Block identifier (-1 for dummy blocks)
     * @param leaf Target leaf position (-1 for dummy blocks)
     * @param data Data content ("dummy" for dummy blocks)
     * @param dummy Whether this is a dummy block (true for dummy blocks)
     */
    block(int id = -1, int leaf = -1, const string& data = "dummy", bool dummy = true);

    /**
     * @brief Prints block information for debugging
     * 
     * Outputs block ID, leaf position, data content, and dummy status.
     */
    void print_block();

    // TODO for rORAM: Add the following fields
    // timestamp_t last_access;    // Last access time for recency tracking
    // size_t block_size;          // Actual size of data for variable sizing
    // int access_count;           // Number of times block has been accessed
};

#endif
