#include <iostream>
#include <fstream>
#include <string>
#include "AVLTree.h"

using namespace std;

// Main function: Handles input, output, and operations on the AVL tree
int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);            // Optimize I/O (Found on stackOverflow)
    cin.tie(nullptr);

    // Ensure correct number of arguments is provided
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input file> <output file>" << endl;
        return 1;
    }

    // Open input and output files
    ifstream infile(argv[1]);
    ofstream outfile(argv[2]);

    // Sanity check (check if files open correctly)
    if (!infile.is_open() || !outfile.is_open()) {
    cerr << "Error opening file(s)." << endl;
    return 1;
    }

    AVLTree tree;
    string command;

    while (infile >> command) {
        if (command == "i") {
            string word;
            infile >> word;
            tree.insert(word);
        } else if (command == "r") {
            string start, end;
            infile >> start >> end;
            outfile << tree.rangeQuery(start, end) << endl;
        }
    }

    infile.close();
    outfile.close();

    return 0;
}