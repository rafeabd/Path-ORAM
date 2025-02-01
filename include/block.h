#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"

using namespace std;

struct block{
	int leaf;
	int id;
	int data[block_size];
	bool dummy;

	block(int, const string, bool);
};

#endif
