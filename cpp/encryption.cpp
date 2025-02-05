#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <vector>
#include <openssl/rand.h>
#include "../include/encryption.h"

std::vector<unsigned char> create_encrypted_id(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, size_t length) {
    std::vector<unsigned char> output(length);
    unsigned int len = length;

    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha256(), NULL);
    HMAC_Update(ctx, data.data(), data.size());
    HMAC_Final(ctx, output.data(), &len);
    HMAC_CTX_free(ctx);

    return output;
}

std::vector<unsigned char> generateEncryptionKey(size_t length) {
    std::vector<unsigned char> key(length);
    if (!RAND_bytes(key.data(), length)) {
        throw std::runtime_error("Error generating random bytes for key");
    }
    return key;
}

std::vector<unsigned char> encryptData(const std::vector<unsigned char>& key, const std::vector<unsigned char>& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
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

std::vector<unsigned char> decryptData(const std::vector<unsigned char>& key, const std::vector<unsigned char>& ciphertext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> plaintext(ciphertext.size());
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
    std::vector<unsigned char> key = generateEncryptionKey(key_length);

    // Data to be encrypted
    std::vector<unsigned char> plaintext = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

    // Encrypt the data
    std::vector<unsigned char> ciphertext = encryptData(key, plaintext);

    // Decrypt the data
    std::vector<unsigned char> decryptedtext = decryptData(key, ciphertext);

    // Create encrypted ID
    std::vector<unsigned char> id_data = {'i', 'd', '_', 'd', 'a', 't', 'a'};
    std::vector<unsigned char> encrypted_id = create_encrypted_id(key, id_data, key_length);

    // Output results
    std::cout << "Plaintext: ";
    for (unsigned char c : plaintext) {
        std::cout << c;
    }
    std::cout << std::endl;

    std::cout << "Ciphertext: ";
    for (unsigned char c : ciphertext) {
        printf("%02x", c);
    }
    std::cout << std::endl;

    std::cout << "Decrypted text: ";
    for (unsigned char c : decryptedtext) {
        std::cout << c;
    }
    std::cout << std::endl;

    std::cout << "Encrypted ID: ";
    for (unsigned char c : encrypted_id) {
        printf("%02x", c);
    }
    std::cout << std::endl;

    return 0;
}
*/