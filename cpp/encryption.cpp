#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <vector>
#include <openssl/rand.h>
#include "../include/encryption.h"

using namespace std;

#include <vector>
#include <stdexcept>
#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

/**
 * @brief Creates an encrypted ID for a block using HMAC (Hash-based Message Authentication Code).
 * 
 * In Path ORAM, encrypted block IDs ensure that the adversary cannot link multiple accesses
 * to the same block. This function uses HMAC-SHA256 to create an encrypted identifier.
 * 
 * @param key The secret encryption key used for HMAC.
 * @param data The input data (e.g., original block ID).
 * @param length The desired length of the output encrypted ID.
 * @return A vector containing the encrypted ID.
 */
vector<unsigned char> create_encrypted_id(const vector<unsigned char>& key, const vector<unsigned char>& data, size_t length) {
    vector<unsigned char> output(length);  // Buffer for the output.
    unsigned int len = length;  // Length variable for the HMAC output.

    // Create a new HMAC context for computing the hash.
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha256(), NULL);  // Initialize HMAC with the given key.
    HMAC_Update(ctx, data.data(), data.size());  // Process the input data.
    HMAC_Final(ctx, output.data(), &len);  // Generate the final HMAC output.
    HMAC_CTX_free(ctx);  // Free the HMAC context.

    return output;
}

/**
 * @brief Generates a cryptographic random key of the given length.
 * 
 * Used in Path ORAM for encryption of block IDs and data to ensure confidentiality.
 * 
 * @param length The size of the encryption key (e.g., 32 bytes for AES-256).
 * @return A randomly generated encryption key.
 */
vector<unsigned char> generateEncryptionKey(size_t length) {
    vector<unsigned char> key(length);  // Allocate space for the key.
    if (!RAND_bytes(key.data(), length)) {  // Generate secure random bytes.
        throw runtime_error("Error generating random bytes for key");  // Handle failure in random key generation.
    }
    return key;
}

/**
 * @brief Encrypts data using AES-256-CBC.
 * 
 * Used in Path ORAM to encrypt stored data blocks, ensuring that even if an attacker
 * accesses the ORAM storage, they cannot read the data without the key.
 * 
 * @param key The encryption key.
 * @param plaintext The plaintext data to be encrypted.
 * @return The encrypted ciphertext.
 */
vector<unsigned char> encryptData(const vector<unsigned char>& key, const vector<unsigned char>& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();  // Create an encryption context.
    vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);  // Buffer for encrypted data.
    int len;
    int ciphertext_len;

    // Initialize AES-256-CBC encryption with the given key.
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), NULL);
    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size());  // Encrypt the data.
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);  // Finalize encryption.
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);  // Free the encryption context.
    ciphertext.resize(ciphertext_len);  // Resize the output to the actual length.
    return ciphertext;
}

/**
 * @brief Decrypts data using AES-256-CBC.
 * 
 * Used in Path ORAM to recover encrypted blocks from storage after fetching them.
 * 
 * @param key The encryption key.
 * @param ciphertext The encrypted data.
 * @return The decrypted plaintext.
 */
vector<unsigned char> decryptData(const vector<unsigned char>& key, const vector<unsigned char>& ciphertext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();  // Create a decryption context.
    vector<unsigned char> plaintext(ciphertext.size());  // Buffer for decrypted data.
    int len;
    int plaintext_len;

    // Initialize AES-256-CBC decryption with the given key.
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), NULL);
    EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());  // Decrypt the data.
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);  // Finalize decryption.
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);  // Free the decryption context.
    plaintext.resize(plaintext_len);  // Resize the output to the actual length.
    return plaintext;
}

/*
 * @brief Test function demonstrating encryption, decryption, and ID hashing.
 *
 * Simulates Path ORAM operations where:
 * - A key is generated.
 * - Data is encrypted and decrypted.
 * - Block IDs are hashed to protect ORAM access patterns.
 */
/*
int main() {
    // Generate a random encryption key (32 bytes for AES-256).
    size_t key_length = 32;
    vector<unsigned char> key = generateEncryptionKey(key_length);

    // Example plaintext data to be encrypted.
    vector<unsigned char> plaintext = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

    // Encrypt the data.
    vector<unsigned char> ciphertext = encryptData(key, plaintext);

    // Decrypt the data.
    vector<unsigned char> decryptedtext = decryptData(key, ciphertext);

    // Create an encrypted ID for block identification in ORAM.
    vector<unsigned char> id_data = {'i', 'd', '_', 'd', 'a', 't', 'a'};
    vector<unsigned char> encrypted_id = create_encrypted_id(key, id_data, key_length);

    // Output plaintext.
    cout << "Plaintext: ";
    for (unsigned char c : plaintext) {
        cout << c;
    }
    cout << endl;

    // Output ciphertext in hexadecimal format.
    cout << "Ciphertext: ";
    for (unsigned char c : ciphertext) {
        printf("%02x", c);
    }
    cout << endl;

    // Output decrypted text.
    cout << "Decrypted text: ";
    for (unsigned char c : decryptedtext) {
        cout << c;
    }
    cout << endl;

    // Output encrypted ID in hexadecimal format.
    cout << "Encrypted ID: ";
    for (unsigned char c : encrypted_id) {
        printf("%02x", c);
    }
    cout << endl;

    return 0;
}
*/
