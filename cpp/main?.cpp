#include <iostream>
#include "../include/block.h"
#include "../include/bucket.h"
#include "../include/bst.h"
#include "../include/client.h"
#include "../include/encryption.h"
using namespace std;

int main() {
    // Generate encryption key
    std::vector<unsigned char> key_for_id = generateEncryptionKey(32);
    std::vector<unsigned char> key_for_data = generateEncryptionKey(32);
    for (unsigned char c : key_for_id) {
        printf("%02x", c);
    }
    return 0;
}