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
#include "../include/bucket.h"

using namespace std;

vector<unsigned char> generateEncryptionKey(size_t length) {
    vector<unsigned char> key(length);
    if (!RAND_bytes(key.data(), length)) {
        throw runtime_error("Error generating random bytes for key");
    }
    return key;
}

// Encodes a vector of unsigned char into a hexadecimal string.
string hexEncode(const vector<unsigned char>& data) {
    ostringstream oss;
    oss << hex << setfill('0');
    for (auto byte : data) {
        oss << setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

// Decodes a hexadecimal string back into a vector of unsigned char.
vector<unsigned char> hexDecode(const string &hex) {
    vector<unsigned char> data;
    for (size_t i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
        data.push_back(byte);
    }
    return data;
}

// Creates an encrypted ID (HMAC)
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

// Randomized encryption
vector<unsigned char> encryptData(const vector<unsigned char>& key, const vector<unsigned char>& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    int iv_length = EVP_CIPHER_iv_length(cipher);
    
    // Generate a random IV
    vector<unsigned char> iv(iv_length);
    if (RAND_bytes(iv.data(), iv_length) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to generate random IV");
    }
    
    // Initialize encryption with random IV
    if(1 != EVP_EncryptInit_ex(ctx, cipher, NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }
    
    // Output buffer, randomized IV+cipher
    vector<unsigned char> ciphertext(iv); 
    ciphertext.resize(iv_length + plaintext.size() + EVP_CIPHER_block_size(cipher));
    
    int len;
    int ciphertext_len = 0;
    
    // Encrypt plaintext
    if(1 != EVP_EncryptUpdate(ctx, ciphertext.data() + iv_length, &len, plaintext.data(), plaintext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    ciphertext_len = len;
    
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + iv_length + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize to the proper output length
    ciphertext.resize(iv_length + ciphertext_len);

    return ciphertext;
}

// Decrypt using the IV from ciphertext
vector<unsigned char> decryptData(const vector<unsigned char>& key, const vector<unsigned char>& ciphertext) {
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    int iv_length = EVP_CIPHER_iv_length(cipher);
    if (ciphertext.size() < (size_t)iv_length) {
        throw std::runtime_error("Ciphertext too short, missing IV");
    }
    
    // Extract IV

    /*
    stringstream ss;
    for (auto it = ciphertext.begin(); it != ciphertext.end(); it++)
        ss << *it;
    cout << ss.str() << endl;
    */
    vector<unsigned char> iv(ciphertext.begin(), ciphertext.begin() + iv_length);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    
    if(1 != EVP_DecryptInit_ex(ctx, cipher, NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }
    
    // Buffer for plaintext
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

// Serialize
string serializeBlock(block &b) {
    //b.print_block();
    ostringstream oss;
    oss << b.id << "|" << (b.dummy ? "1" : "0") << "|";
    
    for (size_t i = 0; i < b.paths.size(); i++) {
        oss << b.paths[i];
        if (i < b.paths.size() - 1) {
            oss << ",";
        }
    }
    oss << "|";
    
    string paddedData = b.data;
    //paddedData.resize(30, ' ');
    oss << paddedData;
    const size_t BLOCK_SIZE = 2020;
    string serialized = oss.str();
    
    if (serialized.size() > BLOCK_SIZE) {
        throw runtime_error("Serialized block exceeds fixed block size");
    }
    serialized.resize(BLOCK_SIZE, ' ');
    //cout << "serialized size of block in serialize block" << serialized.size() << endl;
    return serialized;
}


// Deserialize 
block deserializeBlock(const string &s) {
    istringstream iss(s);
    string token;

    getline(iss, token, '|');
    int id = stoi(token);
    
    getline(iss, token, '|');
    bool dummy = (token == "1");
    
    getline(iss, token, '|');
    vector<int> paths;
    if (!token.empty()) {
        istringstream pathIss(token);
        string pathToken;
        while (getline(pathIss, pathToken, ',')) {
            paths.push_back(stoi(pathToken));
        }
    }
    
    string data;
    getline(iss, data);
    
    while (!data.empty() && data.back() == ' ') {
        data.pop_back();
    }
    
    return block(id, data, dummy, paths);
}

// Encrypt the whole block
block encryptBlock(block &b, const vector<unsigned char>& key) {
    //cout << "in encrypt block" << endl;
    //b.print_block();
    string plaintext = serializeBlock(b);
    vector<unsigned char> plainVec(plaintext.begin(), plaintext.end());
    vector<unsigned char> cipherVec = encryptData(key, plainVec);
    string cipherHex = hexEncode(cipherVec);
    vector<int> empty_path = {};
    //cout << "size of encrypted block" << cipherHex.size() << endl;
    return block(0, cipherHex, false, empty_path);
}

// Decrypt whole block
block decryptBlock(const block &b, const vector<unsigned char>& key) {
    vector<unsigned char> cipherVec = hexDecode(b.data);
    vector<unsigned char> plainVec = decryptData(key, cipherVec);
    string plainText(plainVec.begin(), plainVec.end());
    return deserializeBlock(plainText);
}

string serialize_bucket(Bucket bucket){
    ostringstream oss;
    for (block b : bucket.getBlocks()){
        //cout << "data size: " << b.data.size() << endl;
        oss << b.data;
    }
    //cout <<  "bucket size: " << oss.str().size() << endl;
    return oss.str();
}

Bucket deserialize_bucket(string read_string){
    const size_t blockSize = 4096;
    const size_t expectedSize = 16384;
    
    if (read_string.size() != expectedSize) {
        cout << "read_string: " << read_string.size() << endl;
        throw runtime_error("Bucket data must be exactly 512 characters.");
    }
    
    vector<block> blocks;
    // Loop over the string in increments of 4096.
    for (size_t i = 0; i < expectedSize; i += blockSize) {
        string blockStr = read_string.substr(i, blockSize);
        block block_being_deserialized = block(0,blockStr,false,vector<int>{});
        blocks.push_back(block_being_deserialized);
    }
    Bucket result = Bucket();
    for (block b: blocks){
        result.addBlock(b);
    }
    return result;
}

Bucket encrypt_bucket(Bucket bucket_to_encrypt, const vector<unsigned char>& key){
    for (block &b: bucket_to_encrypt.getBlocks()){
        b = encryptBlock(b,key);
    }
    return bucket_to_encrypt;
}

Bucket decrypt_bucket(Bucket bucket_to_decrypt, const vector<unsigned char>& key){
    for (block &b: bucket_to_decrypt.getBlocks()){
        b = decryptBlock(b,key);
    }
    return bucket_to_decrypt;
}