#include "core/CryptoManager.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

#ifdef HAVE_CRYPTO_LIBS
extern "C" {
#include <argon2.h>
#include <gpgme.h>
}
#endif

namespace crimson {
namespace core {

// PIMPL implementation for CryptoManager
class CryptoManager::Impl {
public:
#ifdef HAVE_CRYPTO_LIBS
    gpgme_ctx_t gpg_ctx;
#endif
    bool initialized;
    
#ifdef HAVE_CRYPTO_LIBS
    Impl() : gpg_ctx(nullptr), initialized(false) {}
    
    ~Impl() {
        if (gpg_ctx) {
            gpgme_release(gpg_ctx);
        }
    }
#else
    Impl() : initialized(false) {}
    ~Impl() = default;
#endif
};

CryptoManager::CryptoManager() : impl_(std::make_unique<Impl>()) {
}

CryptoManager::~CryptoManager() = default;

bool CryptoManager::initialize() {
#ifdef HAVE_CRYPTO_LIBS
    // Initialize GPGME
    gpgme_check_version(nullptr);
    
    gpgme_error_t err = gpgme_new(&impl_->gpg_ctx);
    if (err) {
        return false;
    }
    
    // Set ASCII armor for output
    gpgme_set_armor(impl_->gpg_ctx, 1);
#endif
    
    impl_->initialized = true;
    return true;
}

std::unique_ptr<SecureMemory::SecureBuffer> CryptoManager::deriveKey(
    const std::string& masterPassword,
    std::string& salt) {
    
    if (!impl_->initialized) {
        throw std::runtime_error("CryptoManager not initialized");
    }
    
    // Generate salt if not provided
    if (salt.empty()) {
        salt = generateSalt();
    }
    
    // Decode base64 salt
    std::vector<uint8_t> saltBytes = fromBase64(salt);
    
    // Create buffer for derived key (32 bytes for AES-256)
    auto keyBuffer = SecureMemory::createBuffer(32);
    
#ifdef HAVE_CRYPTO_LIBS
    // Use Argon2id for key derivation
    int ret = argon2id_hash_raw(
        3,                              // time cost (iterations)
        65536,                          // memory cost (64 MB)
        4,                              // parallelism
        masterPassword.c_str(),         // password
        masterPassword.length(),        // password length
        saltBytes.data(),               // salt
        saltBytes.size(),               // salt length
        keyBuffer->as<uint8_t>(),       // output buffer
        keyBuffer->size()               // output length
    );
    
    if (ret != ARGON2_OK) {
        throw std::runtime_error("Key derivation failed: " + std::string(argon2_error_message(ret)));
    }
#else
    // Simplified key derivation using Qt's crypto (NOT SECURE - for development only)
    QString combined = QString::fromStdString(masterPassword) + QString::fromUtf8(reinterpret_cast<const char*>(saltBytes.data()), saltBytes.size());
    
    // Multiple rounds of SHA-256 for basic stretching
    QByteArray key = combined.toUtf8();
    for (int i = 0; i < 10000; ++i) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
    }
    
    std::memcpy(keyBuffer->as<uint8_t>(), key.constData(), std::min(static_cast<int>(keyBuffer->size()), key.size()));
#endif
    
    return keyBuffer;
}

std::vector<uint8_t> CryptoManager::encrypt(
    const std::string& plaintext,
    const SecureMemory::SecureBuffer& key) {
    
    if (!impl_->initialized) {
        throw std::runtime_error("CryptoManager not initialized");
    }
    
    // For simplicity, we'll use symmetric encryption with a temporary key
    // In a real implementation, you might want to create a temporary GPG key
    // or use a different encryption library like libsodium
    
    // This is a simplified implementation - in production you'd want to use
    // proper GPG symmetric encryption or a dedicated crypto library
    
    // Create a simple XOR cipher with the key (NOT SECURE - just for demo)
    // TODO: Implement proper AES-256 encryption
    std::vector<uint8_t> result;
    result.reserve(plaintext.size());
    
    const uint8_t* keyData = key.as<uint8_t>();
    
    for (size_t i = 0; i < plaintext.size(); ++i) {
        result.push_back(plaintext[i] ^ keyData[i % key.size()]);
    }
    
    return result;
}

std::string CryptoManager::decrypt(
    const std::vector<uint8_t>& ciphertext,
    const SecureMemory::SecureBuffer& key) {
    
    if (!impl_->initialized) {
        throw std::runtime_error("CryptoManager not initialized");
    }
    
    // Corresponding decrypt for the simple XOR cipher
    // TODO: Implement proper AES-256 decryption
    std::string result;
    result.reserve(ciphertext.size());
    
    const uint8_t* keyData = key.as<uint8_t>();
    
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        result.push_back(ciphertext[i] ^ keyData[i % key.size()]);
    }
    
    return result;
}

std::string CryptoManager::generateSalt(size_t size) {
    std::vector<uint8_t> salt(size);
    
    // Use system random for salt generation
    std::random_device rd;
    for (size_t i = 0; i < size; ++i) {
        salt[i] = static_cast<uint8_t>(rd());
    }
    
    return toBase64(salt);
}

bool CryptoManager::verifyMasterPassword(
    const std::string& masterPassword,
    const std::string& storedHash,
    const std::string& salt) {
    
    std::string computedHash = hashMasterPassword(masterPassword, salt);
    return computedHash == storedHash;
}

std::string CryptoManager::hashMasterPassword(
    const std::string& masterPassword,
    const std::string& salt) {
    
    std::vector<uint8_t> saltBytes = fromBase64(salt);
    
#ifdef HAVE_CRYPTO_LIBS
    std::vector<uint8_t> hash(32);
    
    int ret = argon2id_hash_raw(
        3,                              // time cost
        65536,                          // memory cost (64 MB)
        4,                              // parallelism
        masterPassword.c_str(),         // password
        masterPassword.length(),        // password length
        saltBytes.data(),               // salt
        saltBytes.size(),               // salt length
        hash.data(),                    // output buffer
        hash.size()                     // output length
    );
    
    if (ret != ARGON2_OK) {
        throw std::runtime_error("Password hashing failed: " + std::string(argon2_error_message(ret)));
    }
    
    return toBase64(hash);
#else
    // Simplified password hashing using Qt's crypto (NOT SECURE - for development only)
    QString combined = QString::fromStdString(masterPassword) + QString::fromUtf8(reinterpret_cast<const char*>(saltBytes.data()), saltBytes.size());
    
    QByteArray hash = combined.toUtf8();
    for (int i = 0; i < 10000; ++i) {
        hash = QCryptographicHash::hash(hash, QCryptographicHash::Sha256);
    }
    
    return toBase64(std::vector<uint8_t>(hash.begin(), hash.end()));
#endif
}

std::string CryptoManager::sha256(const std::string& data) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data.c_str(), data.length());
    return hash.result().toHex().toStdString();
}

std::string CryptoManager::toBase64(const std::vector<uint8_t>& data) {
    QByteArray qdata(reinterpret_cast<const char*>(data.data()), data.size());
    return qdata.toBase64().toStdString();
}

std::vector<uint8_t> CryptoManager::fromBase64(const std::string& base64) {
    QByteArray qdata = QByteArray::fromBase64(base64.c_str());
    std::vector<uint8_t> result(qdata.size());
    std::memcpy(result.data(), qdata.data(), qdata.size());
    return result;
}

} // namespace core
} // namespace crimson
