#include "server.h"
#include <cmath>
#include <algorithm>

using namespace std;

Server::Server(int num_blocks, int bucket_size)
    : L(ceil(log2(num_blocks))), Z(bucket_size) {
    int tree_size = (1 << (L + 1)) - 1;
    tree.resize(tree_size, Bucket(Z));
    initializeWithDummyBlocks();
}

vector<Block> Server::readPath(int leaf) {
    vector<Block> path_blocks;
    vector<int> path = getPathToLeaf(leaf);

    for (int node : path) {
        auto blocks = tree[node].removeAllBlocks();
        path_blocks.insert(path_blocks.end(), blocks.begin(), blocks.end());
    }

    return path_blocks;
}

void Server::writePath(int leaf, const vector<Block>& blocks) {
    vector<int> path = getPathToLeaf(leaf);

    // Clear path buckets
    for (int node : path) {
        tree[node].removeAllBlocks();
    }

    // Write blocks to path
    for (const Block& block : blocks) {
        for (int node : path) {
            if (tree[node].hasSpace()) {
                tree[node].addBlock(block);
                break;
            }
        }
    }

    // Fill with dummy blocks
    for (int node : path) {
        while (tree[node].hasSpace()) {
            tree[node].addBlock(Block());
        }
    }
}

vector<int> Server::getPathToLeaf(int leaf) const {
    vector<int> path;
    int node = leaf + ((1 << L) - 1);

    while (node >= 0) {
        path.push_back(node);
        node = (node - 1) / 2;
    }

    reverse(path.begin(), path.end());
    return path;
}

void Server::initializeWithDummyBlocks() {
    for (auto& bucket : tree) {
        while (bucket.hasSpace()) {
            bucket.addBlock(Block());
        }
    }
}