#include "helper.h"

using namespace std;

int toNormalIndex(int physicalIndex) {
    // get level of physical index
    int level = 0;
    int temp = physicalIndex + 1;
    while (temp >>= 1){
        level++;
    }
    int levelStart = (1 << level) - 1;

    // get position in the level
    int pos_br = physicalIndex - levelStart;
    //cout << "physcial Index: " << physicalIndex << endl;
    //cout << "level start: " << levelStart << endl;
    // get bitreverse version - returns normal)
    int pos_normal = bitReverse(pos_br, level);
    return levelStart + pos_normal;
}

int toPhysicalIndex(int normalIndex) {
    int level = 0;
    int temp = normalIndex + 1;
    while (temp >>= 1){
        level++;
    }
    int levelStart = (1 << level) - 1;
    int pos_normal = normalIndex - levelStart;
    //cout << "normalIndex: " << normalIndex << endl;
    //cout << "level start: " << levelStart << endl;
    int pos_br = bitReverse(pos_normal, level);
    return levelStart + pos_br;
}


int bitReverse(int x, int bits) {
    int y = 0;
    for (int i = 0; i < bits; i++) {
        y = (y << 1) | (x & 1);
        x >>= 1;
    }
    return y;
}