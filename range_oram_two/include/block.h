#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <vector>

using namespace std;

class block {
public:
    int id;
    int leaf;
    string data;
    bool dummy;
    vector<int> paths;  // Store path tags for all ORAMs (p0...pℓ)
    
    // Default constructor
    block() : id(-1), leaf(-1), data(""), dummy(true), paths() {}
    
    // Constructor with path tags for all ORAMs
    block(int id, int leaf, const string& data, bool dummy, const vector<int>& paths = vector<int>());
    
    void print_block(bool print_paths = false);
};

#endif // BLOCK_H