#ifndef BUCKET_H
#define BUCKET_H

#include "block.h"
#include <vector>

struct bucket {
    std::vector<block> blocks;

    bucket(int bucket_size);
};

#endif