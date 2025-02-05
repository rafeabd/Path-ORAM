#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>

std::vector<unsigned char> create_encrypted_id(
    const std::vector<unsigned char>& key, 
    const std::vector<unsigned char>& data, 
    size_t length);

std::vector<unsigned char> generateEncryptionKey(size_t length);

std::vector<unsigned char> encryptData(
    const std::vector<unsigned char>& key, 
    const std::vector<unsigned char>& plaintext);

std::vector<unsigned char> decryptData(
    const std::vector<unsigned char>& key, 
    const std::vector<unsigned char>& ciphertext);

#endif