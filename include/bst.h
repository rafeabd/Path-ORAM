#ifndef HEAP_H
#define HEAP_H

#include <string>
#include "block.h"

using namespace std;

class BinHeap
{
	private:
        int* A; // it's an array of ints
        int size; // a non-negative size
        int capacity; // a non-negative capacity

	public:
        BinHeap(); // Default constructor
        BinHeap(int cap); // Constructor to set capacity of heap
        BinHeap(int* init, int new_size, int cap); // Initialize the binary heap to an array
        void resize(int cap); // resize the underlying array to new capacity
        void deleteHeap(); // frees the array A
        void insert(block,int); // insert int into heap
        string print(); // print the heap in array order into a string
        vector<int> extractPath(int leafIndex);
		
        
};

#endif
