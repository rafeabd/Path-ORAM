#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>
#include "block.h"
#include "bucket.h"

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

string hexEncode(const vector<unsigned char>& data);
vector<unsigned char> hexDecode(const string &hex);
string serializeBlock(block &b);
block deserializeBlock(const string &s);

string serialize_bucket(Bucket bucket);
Bucket deserialize_bucket(string read_string);

block encryptBlock(block &b, const vector<unsigned char>& key);
block decryptBlock(const block &b, const vector<unsigned char>& key);

Bucket encrypt_bucket(Bucket bucket_to_encrypt, const vector<unsigned char>& key);
Bucket decrypt_bucket(Bucket bucket_to_encrypt, const vector<unsigned char>& key);

#endif