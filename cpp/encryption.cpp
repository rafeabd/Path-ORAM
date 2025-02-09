#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <vector>
#include <openssl/rand.h>
#include "../include/encryption.h"

using namespace std;

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

vector<unsigned char> generateEncryptionKey(size_t length) {
    vector<unsigned char> key(length);
    if (!RAND_bytes(key.data(), length)) {
        throw runtime_error("Error generating random bytes for key");
    }
    return key;
}

vector<unsigned char> encryptData(const vector<unsigned char>& key, const vector<unsigned char>& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len;
    int ciphertext_len;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), NULL);
    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

vector<unsigned char> decryptData(const vector<unsigned char>& key, const vector<unsigned char>& ciphertext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    vector<unsigned char> plaintext(ciphertext.size());
    int len;
    int plaintext_len;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), NULL);
    EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(plaintext_len);
    return plaintext;
}

/*
int main() {
    // Generate encryption key
    size_t key_length = 32;
    vector<unsigned char> key = generateEncryptionKey(key_length);

    // Data to be encrypted
    vector<unsigned char> plaintext = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

    // Encrypt the data
    vector<unsigned char> ciphertext = encryptData(key, plaintext);

    // Decrypt the data
    vector<unsigned char> decryptedtext = decryptData(key, ciphertext);

    // Create encrypted ID
    vector<unsigned char> id_data = {'i', 'd', '_', 'd', 'a', 't', 'a'};
    vector<unsigned char> encrypted_id = create_encrypted_id(key, id_data, key_length);

    // Output results
    cout << "Plaintext: ";
    for (unsigned char c : plaintext) {
        cout << c;
    }
    cout << endl;

    cout << "Ciphertext: ";
    for (unsigned char c : ciphertext) {
        printf("%02x", c);
    }
    cout << endl;

    cout << "Decrypted text: ";
    for (unsigned char c : decryptedtext) {
        cout << c;
    }
    cout << endl;

    cout << "Encrypted ID: ";
    for (unsigned char c : encrypted_id) {
        printf("%02x", c);
    }
    cout << endl;

    return 0;
}
*/