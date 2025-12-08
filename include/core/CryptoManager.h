#pragma once

#include <string>
#include <vector>
#include <memory>
#include "SecureMemory.h"

namespace crimson {
namespace core {

/**
 * @brief Cryptographic operations manager
 * 
 * Handles all encryption/decryption operations using GPG and Argon2.
 * Provides secure key derivation and AES-256 encryption through GPG.
 */
class CryptoManager {
public:
    CryptoManager();
    ~CryptoManager();
    
    /**
     * @brief Initialize cryptographic context
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Derive encryption key from master password
     * @param masterPassword Master password
     * @param salt Salt for key derivation (if empty, generates new)
     * @return Derived key in secure memory
     */
    std::unique_ptr<SecureMemory::SecureBuffer> deriveKey(
        const std::string& masterPassword,
        std::string& salt);
    
    /**
     * @brief Encrypt data using derived key
     * @param plaintext Data to encrypt
     * @param key Encryption key
     * @return Encrypted data in GPG format
     */
    std::vector<uint8_t> encrypt(
        const std::string& plaintext,
        const SecureMemory::SecureBuffer& key);
    
    /**
     * @brief Decrypt data using derived key
     * @param ciphertext Encrypted data
     * @param key Decryption key
     * @return Decrypted plaintext
     */
    std::string decrypt(
        const std::vector<uint8_t>& ciphertext,
        const SecureMemory::SecureBuffer& key);
    
    /**
     * @brief Generate cryptographically secure salt
     * @param size Salt size in bytes (default: 32)
     * @return Base64-encoded salt
     */
    std::string generateSalt(size_t size = 32);
    
    /**
     * @brief Verify master password against stored hash
     * @param masterPassword Password to verify
     * @param storedHash Stored password hash
     * @param salt Salt used for hashing
     * @return true if password is correct
     */
    bool verifyMasterPassword(
        const std::string& masterPassword,
        const std::string& storedHash,
        const std::string& salt);
    
    /**
     * @brief Hash master password for storage
     * @param masterPassword Password to hash
     * @param salt Salt for hashing
     * @return Base64-encoded hash
     */
    std::string hashMasterPassword(
        const std::string& masterPassword,
        const std::string& salt);
    
    /**
     * @brief Calculate SHA256 hash
     * @param data Data to hash
     * @return Hex-encoded hash
     */
    static std::string sha256(const std::string& data);

    /**
     * @brief Convert binary data to base64
     */
    static std::string toBase64(const std::vector<uint8_t>& data);
    
    /**
     * @brief Convert base64 to binary data
     */
    static std::vector<uint8_t> fromBase64(const std::string& base64);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core
} // namespace crimson
