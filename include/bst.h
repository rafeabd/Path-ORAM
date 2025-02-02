// Filename: linkedlist.h
// 
// Header file for the class BinHeap that represents a Binary Heap 
// 
// C. Seshadhri, Jan 2020

#ifndef HEAP_H
#define HEAP_H

#include <string>

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
        int findMax(); // output the maximum value
        void swim(int index); // "swim" a high priority to the top of the heap
        void insert(int); // insert int into heap
        string print(); // print the heap in array order into a string
		void heapify(int);
		void extractMax(void);
        vector<int> extractPath(int leafIndex);
		
        
};

#endif
