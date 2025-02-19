/**
 * @file encryption.cpp
 * @brief Implements encryption functionality for Path ORAM
 * 
 * This file provides the implementation of encryption operations,
 * including block encryption/decryption, key generation, and data
 * serialization.
 * 
 * For rORAM extension:
 * 1. Optimize encryption for different storage levels
 * 2. Add compression support
 * 3. Implement hierarchical encryption
 * 4. Add key rotation mechanisms
 */

#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <iomanip>

#include "../include/encryption.h"
#include "../include/block.h"

using namespace std;

/**
 * @brief Converts binary data to hex string
 */
string hexEncode(const vector<unsigned char>& data) {
    ostringstream oss;
    oss << hex << setfill('0');
    for (auto byte : data) {
        oss << setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

/**
 * @brief Converts hex string to binary data
 */
vector<unsigned char> hexDecode(const string &hex) {
    vector<unsigned char> data;
    for (size_t i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
        data.push_back(byte);
    }
    return data;
}

/**
 * @brief Creates encrypted ID (currently not used)
 */
vector<unsigned char> create_encrypted_id(const vector<unsigned char>& key, const vector<unsigned char>& data, size_t length) {
    vector<unsigned char> output(length);
    unsigned int len = length;

    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha256(), NULL);
    HMAC_Update(ctx, data.data(), data.size());
    HMAC_Final(ctx, output.data(), &len);
    HMAC_CTX_free(ctx);

    return output;
}

/**
 * @brief Generates encryption key
 * 
 * For rORAM:
 * - Add support for multiple keys
 * - Implement key hierarchy
 */
vector<unsigned char> generateEncryptionKey(size_t length) {
    vector<unsigned char> key(length);
    if (!RAND_bytes(key.data(), length)) {
        throw runtime_error("Error generating random bytes for key");
    }
    return key;
}

/**
 * @brief Encrypts data using AES-256-CBC
 * 
 * For rORAM:
 * - Add compression before encryption
 * - Implement level-specific encryption
 */
vector<unsigned char> encryptData(const vector<unsigned char>& key, const vector<unsigned char>& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    int iv_length = EVP_CIPHER_iv_length(cipher);
    
    // Generate a random #
    vector<unsigned char> iv(iv_length);
    RAND_bytes(iv.data(), iv_length);
    EVP_EncryptInit_ex(ctx, cipher, NULL, key.data(), iv.data());
    
    // output buf, randomized #+cipher
    vector<unsigned char> ciphertext(iv); // Prepend IV.
    ciphertext.resize(iv_length + plaintext.size() + EVP_CIPHER_block_size(cipher));
    
    int len;
    int ciphertext_len = 0;
    
    EVP_EncryptUpdate(ctx, ciphertext.data() + iv_length, &len, plaintext.data(), plaintext.size());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + iv_length + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    ciphertext.resize(iv_length + ciphertext_len);
    return ciphertext;
}

/**
 * @brief Decrypts data using AES-256-CBC
 */
vector<unsigned char> decryptData(const vector<unsigned char>& key, const vector<unsigned char>& ciphertext) {
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    int iv_length = EVP_CIPHER_iv_length(cipher);
    if (ciphertext.size() < (size_t)iv_length) {
        throw std::runtime_error("Ciphertext too short, missing IV");
    }
    
    vector<unsigned char> iv(ciphertext.begin(), ciphertext.begin() + iv_length);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    
    if(1 != EVP_DecryptInit_ex(ctx, cipher, NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }
    
    vector<unsigned char> plaintext(ciphertext.size() - iv_length); 
    int len;
    int plaintext_len = 0;
    
    if(1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data() + iv_length, ciphertext.size() - iv_length)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }
    plaintext_len = len;
    
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    plaintext.resize(plaintext_len);
    return plaintext;
}

/**
 * @brief Serializes block to string
 * 
 * For rORAM:
 * - Add compression
 * - Support variable block sizes
 * - Include metadata
 */
string serializeBlock(const block &b) {
    ostringstream oss;
    oss << b.id << "|" << b.leaf << "|" << (b.dummy ? "1" : "0") << "|";
    // Pad data to fixed 20 characters
    string paddedData = b.data;
    paddedData.resize(20, ' ');
    oss << paddedData;
    return oss.str();
}

/**
 * @brief Deserializes string to block
 */
block deserializeBlock(const string &s) {
    istringstream iss(s);
    string token;
    getline(iss, token, '|');
    int id = stoi(token);
    getline(iss, token, '|');
    int leaf = stoi(token);
    getline(iss, token, '|');
    bool dummy = (token == "1");
    string data;
    getline(iss, data);
    // Remove padding spaces
    while (!data.empty() && data.back() == ' ') {
        data.pop_back();
    }
    return block(id, leaf, data, dummy);
}

/**
 * @brief Encrypts entire block
 * 
 * For rORAM:
 * - Add level-specific encryption
 * - Implement compression
 */
block encryptBlock(const block &b, const vector<unsigned char>& key) {
    string plaintext = serializeBlock(b);
    vector<unsigned char> plainVec(plaintext.begin(), plaintext.end());
    vector<unsigned char> cipherVec = encryptData(key, plainVec);
    string cipherHex = hexEncode(cipherVec);
    return block(0, 0, cipherHex, true);
}

/**
 * @brief Decrypts entire block
 */
block decryptBlock(const block &b, const vector<unsigned char>& key) {
    vector<unsigned char> cipherVec = hexDecode(b.data);
    vector<unsigned char> plainVec = decryptData(key, cipherVec);
    string plainText(plainVec.begin(), plainVec.end());
    return deserializeBlock(plainText);
}
