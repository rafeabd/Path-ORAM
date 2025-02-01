#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"

using namespace std;

struct block{
	//block has a mapping to leaf node, id for identification,
	//data to store the data, dummy to check if the block is dummy
	int leaf;
	int id;
	int data[block_size];
	bool dummy;

	//constructor for initializing block
	block(int, const string, bool);
};

#endif
