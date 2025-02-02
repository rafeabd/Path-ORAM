// Filename: binheap.cpp
// 
// Contains the class BinHeap that represents a binary heap. This file contains basic heap operations.
// 
// C. Seshadhri, Feb 2020

#include "binheap.h"
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

// Return max function. Trivial, simply return A[1], which is maximum by heap property
// Input: void
// Output: int that is maximum value
int BinHeap :: findMax()
{
    // should be careful here. If size is 0, this returns garbage, but it won't throw an error
    // because of default capacity. We will just return A[1], and expect the caller to be careful
    return A[1];
}

// This function "pushes" a large value towards the root, by recursively swapping with the parent. Typically, one assumes that there is a problem at this
// index, and everything else is a heap.
// Input: int index
// Output: void, basically fixes the heap property at index
void BinHeap :: swim(int index)
{
    if(index < 1) // BAD, shouldn't happen
        return;
    if(index == 1 || A[index] <= A[index/2]) // Base case: when index is 1. Or when A[index] is at most parent value
        return;
    // we need to swap index with parent
    int swap = A[index];
    A[index] = A[index/2]; 
    A[index/2] = swap;
    //recursive call at parent
    swim(index/2);
}

// Insert(int val): Inserts the int val into heap. Basically, add it to the end of the array, then call swim
// Input: Int to insert into the heap
// Output: Void, just insert into heap
void BinHeap :: insert(int val)
{
    if(size > capacity) // serious problem. This shouldn't happen. Throw error and exit
    {
        throw std::runtime_error("Error: size of heap is more than capacity.\n Size = "+to_string(size)+", capacity = "+to_string(capacity));
    }
    if(size == capacity) // we cannot add any more to the heap, so resize by doubling
        resize(2*capacity);
    A[size+1] = val; // put value at the end of the array
    swim(size+1); // call swim at the (new) end to fix
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

