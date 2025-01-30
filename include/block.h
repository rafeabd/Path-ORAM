#ifndef BLOCK_H
#define BLOCK_H

#include "config.h"

struct block{
	int leaf;
	int id;
	int data[block_size];

	block();
};

#endif
