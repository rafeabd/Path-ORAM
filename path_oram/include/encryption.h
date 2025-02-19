/**
 * @file encryption.h
 * @brief Defines encryption functionality for Path ORAM
 * 
 * This file provides the encryption interface used throughout the ORAM system.
 * It includes functions for block encryption/decryption, key generation,
 * and block serialization.
 * 
 * For rORAM extension:
 * 1. Add support for hierarchical encryption
 * 2. Implement level-specific encryption
 * 3. Add compression support
 * 4. Optimize for different storage levels
 */

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>
#include "block.h"

using namespace std;

/**
 * @brief Creates encrypted identifier
 * @param key Encryption key
 * @param data Data to encrypt
 * @param length Output length
 * @return Encrypted identifier
 */
vector<unsigned char> create_encrypted_id(
    const vector<unsigned char>& key, 
    const vector<unsigned char>& data, 
    size_t length);

/**
 * @brief Generates new encryption key
 * @param length Key length
 * @return Generated key
 * 
 * For rORAM:
 * - Add support for multiple keys
 * - Implement key rotation
 * - Add level-specific keys
 */
vector<unsigned char> generateEncryptionKey(size_t length);

/**
 * @brief Encrypts data using AES-256-CBC
 * @param key Encryption key
 * @param plaintext Data to encrypt
 * @return Encrypted data
 */
vector<unsigned char> encryptData(
    const vector<unsigned char>& key, 
    const vector<unsigned char>& plaintext);

/**
 * @brief Decrypts data using AES-256-CBC
 * @param key Encryption key
 * @param ciphertext Data to decrypt
 * @return Decrypted data
 */
vector<unsigned char> decryptData(
    const vector<unsigned char>& key, 
    const vector<unsigned char>& ciphertext);

/**
 * @brief Converts binary data to hex string
 * @param data Binary data
 * @return Hex string
 */
string hexEncode(const vector<unsigned char>& data);

/**
 * @brief Converts hex string to binary data
 * @param hex Hex string
 * @return Binary data
 */
vector<unsigned char> hexDecode(const string &hex);

/**
 * @brief Converts block to string representation
 * @param b Block to serialize
 * @return Serialized block string
 * 
 * For rORAM:
 * - Add compression
 * - Support variable block sizes
 * - Include metadata
 */
string serializeBlock(const block &b);

/**
 * @brief Reconstructs block from string
 * @param s Serialized block string
 * @return Reconstructed block
 */
block deserializeBlock(const string &s);

/**
 * @brief Encrypts entire block
 * @param b Block to encrypt
 * @param key Encryption key
 * @return Encrypted block
 */
block encryptBlock(const block &b, const vector<unsigned char>& key);

/**
 * @brief Decrypts entire block
 * @param b Block to decrypt
 * @param key Encryption key
 * @return Decrypted block
 */
block decryptBlock(const block &b, const vector<unsigned char>& key);

/* TODO for rORAM - Additional functions to consider:
 *
 * struct EncryptionLevel {
 *     vector<unsigned char> level_key;
 *     int compression_level;
 *     bool enable_caching;
 * };
 *
 * vector<unsigned char> compressBlock(const block &b);
 * block decompressBlock(const vector<unsigned char> &data);
 * void rotateKeys(vector<EncryptionLevel> &levels);
 */

#endif
