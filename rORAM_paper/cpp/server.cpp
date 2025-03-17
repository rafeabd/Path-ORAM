
#include "../include/server.h"
#include "../include/bucket.h"
#include "../include/oram.h"
#include "../include/block.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

Server::Server(int num_blocks, int bucket_size, int biggest_range){
    this->Z = bucket_size;
    this->L = ceil(log2(num_blocks));
}
