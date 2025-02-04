#include <iostream>
#include "block.cpp"
#include "bucket.cpp"
#include "bst.cpp"
#include "client.cpp"
#include "encryption.cpp"

int main() {
    // Generate encryption key
    std::vector<unsigned char> key_for_id = generateEncryptionKey(32);
    std::vector<unsigned char> key_for_data = generateEncryptionKey(32);
    return 0;
}