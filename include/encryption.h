#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>

using namespace std;

vector<unsigned char> create_encrypted_id(
    const vector<unsigned char>& key, 
    const vector<unsigned char>& data, 
    size_t length);

vector<unsigned char> generateEncryptionKey(size_t length);

vector<unsigned char> encryptData(
    const vector<unsigned char>& key, 
    const vector<unsigned char>& plaintext);

vector<unsigned char> decryptData(
    const vector<unsigned char>& key, 
    const vector<unsigned char>& ciphertext);

#endif