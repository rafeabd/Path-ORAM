#include "../include/bst.h"
#include "../include/block.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

#define DEFAULT_CAPACITY 1000 // setting a default capacity

// Default constructor sets capacity at default
BinHeap :: BinHeap()
{
    A = new int[DEFAULT_CAPACITY+1]; // the extra +1 is because A[0] will not store anything
    size = 0;
    capacity = DEFAULT_CAPACITY;
}

// Constructor that sets capacity to argument provided
BinHeap :: BinHeap(int cap)
{
    A = new int[DEFAULT_CAPACITY];
    size = 0;
    capacity = cap;
}

// Constructor that initializes heap to input array and capacity
// VERY IMPORTANT: the heap keeps the first element null, so the array must be indexable to its size+1.
// In other words, the input array should have capacity one more than the size, so that the heap works properly.
// Input: An array that we wish to convert into a heap, int size, and int cap (the capacity)
BinHeap :: BinHeap(int init[], int new_size, int cap)
{
    A = init;
    size = new_size;
    capacity = cap;
    for(int i=size; i>0; i--) // Loop backwards over the array, and move every element to the right
        A[i] = A[i-1];
}

// Just deletes heap by freeing memory
void BinHeap :: deleteHeap()
{
    delete(A);
    return;
}

// Resizes the heap to the new capacity. Note that new capacity can be smaller.
// It simply creates a new array, and copies everything up to the size
// Input: int new_capacity
// Output: void, just resizes the array
void BinHeap :: resize(int new_capacity)
{
    if (new_capacity < DEFAULT_CAPACITY) // never go below the default capacity
        new_capacity = DEFAULT_CAPACITY; 
    int* old = A; // store pointer to the old array
    A = new int[new_capacity+1]; // allocate memory for the resizes heap, note the +1 since A[0] does not store anything
    capacity = new_capacity; // reset capacity
    // now we just copy over the old heap to A
    for(int i=1; i<size+1; i++)
        A[i] = old[i];
    delete(old); // free the old heap
}


// Insert(int val): Inserts the int val into heap. Basically, add it to the end of the array, then call swim
// Input: Int to insert into the heap
// Output: Void, just insert into heap
void BinHeap :: insert(block block, int leaf)
{
    if(size > capacity) // serious problem. This shouldn't happen. Throw error and exit
    {
        throw std::runtime_error("Error: size of heap is more than capacity.\n Size = "+to_string(size)+", capacity = "+to_string(capacity));
    }
    if(size == capacity) // we cannot add any more to the heap, so resize by doubling
        resize(2*capacity);
    size++; // increment the size
    return;
}

// Prints heap in order, as if an array. This is for error/demonstration purposes
// Input: None
// Output: string that has all elements of the heap in array order
string BinHeap :: print()
{
    string heap_str = ""; // string that has list
    if(size < 1) // heap is empty
        return heap_str; // return right now, to prevent problems with pop_back later
    for(int i=1; i<size+1; i++)
        heap_str = heap_str+to_string(A[i])+" "; // append next int to the string
    heap_str.pop_back(); // just remove the last space
    return heap_str;
}

void BinHeap::heapify(int index) {
    int largest=index;
    int left=2*index;
    int right=2*index+1;

    if(A[left]>A[largest] && left <= size ){
        largest=left;
    }
    if(A[right]>A[largest] && right <= size){
        largest=right;
    }
    if(largest!=index){
        swap(A[index],A[largest]);
        heapify(largest);
    }
}

void BinHeap :: extractMax(){
	if(size==0){
		return;
	}
	int max = A[1];
	A[1] = A[size--];
	heapify(1);
}

vector<int> BinHeap :: extractPath(int leafIndex){
    vector<int> path;
    while(leafIndex>1){
        path.push_back(A[leafIndex]);
        leafIndex/=2;
    }
    path.push_back(A[1]);
    reverse(path.begin(),path.end());
    return path;
}



