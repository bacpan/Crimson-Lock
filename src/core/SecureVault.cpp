#include "core/SecureVault.h"
#include <fstream>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <algorithm>
#include <stdexcept>

namespace crimson {
namespace core {

SecureVault::SecureVault() 
    : crypto_manager_(std::make_unique<CryptoManager>())
    , password_generator_(std::make_unique<PasswordGenerator>())
    , vault_key_(nullptr)
    , is_open_(false)
    , last_activity_(std::chrono::steady_clock::now())
    , auto_lock_timeout_(60) {
    
    if (!crypto_manager_->initialize()) {
        throw std::runtime_error("Failed to initialize cryptographic manager");
    }
}

SecureVault::~SecureVault() {
    closeVault();
}

bool SecureVault::createVault(const std::string& masterPassword, const std::string& vaultPath) {
    if (masterPassword.empty()) {
        return false;
    }
    
    try {
        vault_path_ = vaultPath;
        
        // Generate salt for key derivation
        vault_salt_ = crypto_manager_->generateSalt();
        
        // Derive key from master password
        vault_key_ = crypto_manager_->deriveKey(masterPassword, vault_salt_);
        
        // Hash master password for verification
        master_hash_ = crypto_manager_->hashMasterPassword(masterPassword, vault_salt_);
        
        // Clear entries and initialize
        entries_.clear();
        is_open_ = true;
        updateActivity();
        
        // Save initial empty vault
        return saveVaultFile();
        
    } catch (const std::exception&) {
        closeVault();
        return false;
    }
}

bool SecureVault::openVault(const std::string& masterPassword, const std::string& vaultPath) {
    if (masterPassword.empty()) {
        return false;
    }
    
    try {
        vault_path_ = vaultPath;
        
        // Load vault file first to get salt and hash
        if (!loadVaultFile()) {
            return false;
        }
        
        // Verify master password
        if (!crypto_manager_->verifyMasterPassword(masterPassword, master_hash_, vault_salt_)) {
            closeVault();
            return false;
        }
        
        // Derive key from master password
        vault_key_ = crypto_manager_->deriveKey(masterPassword, vault_salt_);
        
        is_open_ = true;
        updateActivity();
        
        return true;
        
    } catch (const std::exception&) {
        closeVault();
        return false;
    }
}

void SecureVault::closeVault() {
    clearSensitiveData();
    entries_.clear();
    vault_path_.clear();
    vault_salt_.clear();
    master_hash_.clear();
    vault_key_.reset();
    is_open_ = false;
}

VaultEntry SecureVault::createEntry(const std::string& label) {
    if (!is_open_ || label.empty()) {
        throw std::runtime_error("Vault not open or invalid label");
    }
    
    updateActivity();
    
    VaultEntry entry;
    entry.id = VaultEntry::generateUuid();
    entry.label = label;
    entry.username = password_generator_->generateUsername();
    entry.password = password_generator_->generatePassword();
    entry.created_at = VaultEntry::getCurrentTimestamp();
    entry.device_fingerprint = VaultEntry::getDeviceFingerprint();
    
    return entry;
}

bool SecureVault::saveEntry(const VaultEntry& entry) {
    if (!is_open_) {
        return false;
    }
    
    try {
        updateActivity();
        
        // Encrypt the password before storing
        VaultEntry encryptedEntry = entry;
        std::vector<uint8_t> encryptedPassword = crypto_manager_->encrypt(entry.password, *vault_key_);
        encryptedEntry.password = crypto_manager_->toBase64(encryptedPassword);
        
        // Add or update entry
        auto it = std::find_if(entries_.begin(), entries_.end(),
            [&entry](const VaultEntry& e) { return e.id == entry.id; });
        
        if (it != entries_.end()) {
            *it = encryptedEntry;
        } else {
            entries_.push_back(encryptedEntry);
        }
        
        return saveVaultFile();
        
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<std::pair<std::string, std::string>> SecureVault::getEntryLabels() const {
    if (!is_open_) {
        return {};
    }
    
    std::vector<std::pair<std::string, std::string>> labels;
    labels.reserve(entries_.size());
    
    for (const auto& entry : entries_) {
        labels.emplace_back(entry.id, entry.label);
    }
    
    return labels;
}

VaultEntry SecureVault::getEntry(const std::string& entryId) const {
    if (!is_open_) {
        throw std::runtime_error("Vault not open");
    }
    
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&entryId](const VaultEntry& e) { return e.id == entryId; });
    
    if (it == entries_.end()) {
        throw std::runtime_error("Entry not found");
    }
    
    return *it;
}

std::string SecureVault::getPassword(const std::string& entryId) {
    if (!is_open_) {
        throw std::runtime_error("Vault not open");
    }
    
    updateActivity();
    
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&entryId](const VaultEntry& e) { return e.id == entryId; });
    
    if (it == entries_.end()) {
        throw std::runtime_error("Entry not found");
    }
    
    try {
        // Decrypt the password
        std::vector<uint8_t> encryptedPassword = crypto_manager_->fromBase64(it->password);
        return crypto_manager_->decrypt(encryptedPassword, *vault_key_);
        
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to decrypt password");
    }
}

bool SecureVault::deleteEntry(const std::string& entryId) {
    if (!is_open_) {
        return false;
    }
    
    updateActivity();
    
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&entryId](const VaultEntry& e) { return e.id == entryId; });
    
    if (it == entries_.end()) {
        return false;
    }
    
    entries_.erase(it);
    return saveVaultFile();
}

void SecureVault::setAutoLockTimeout(int timeoutSeconds) {
    auto_lock_timeout_ = timeoutSeconds;
}

bool SecureVault::shouldAutoLock() const {
    if (!is_open_ || auto_lock_timeout_ <= 0) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_activity_);
    
    return elapsed.count() >= auto_lock_timeout_;
}

void SecureVault::updateActivity() {
    last_activity_ = std::chrono::steady_clock::now();
}

bool SecureVault::verifyIntegrity() const {
    // For now, just check if vault file exists and can be read
    // In a full implementation, you would verify checksums and signatures
    if (vault_path_.empty()) {
        return false;
    }
    
    std::ifstream file(vault_path_);
    return file.good();
}

SecureVault::VaultStats SecureVault::getStats() const {
    VaultStats stats;
    stats.entryCount = entries_.size();
    stats.deviceFingerprint = VaultEntry::getDeviceFingerprint();
    
    if (!entries_.empty()) {
        // Find earliest created_at
        auto earliest = std::min_element(entries_.begin(), entries_.end(),
            [](const VaultEntry& a, const VaultEntry& b) {
                return a.created_at < b.created_at;
            });
        stats.createdAt = earliest->created_at;
        
        // Find latest created_at
        auto latest = std::max_element(entries_.begin(), entries_.end(),
            [](const VaultEntry& a, const VaultEntry& b) {
                return a.created_at < b.created_at;
            });
        stats.lastModified = latest->created_at;
    }
    
    return stats;
}

bool SecureVault::loadVaultFile() {
    std::ifstream file(vault_path_);
    if (!file.good()) {
        return false;
    }
    
    try {
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(content), &error);
        
        if (error.error != QJsonParseError::NoError) {
            return false;
        }
        
        QJsonObject root = doc.object();
        
        // Load vault metadata
        vault_salt_ = root["salt"].toString().toStdString();
        master_hash_ = root["master_hash"].toString().toStdString();
        
        // Load entries
        QJsonArray entriesArray = root["entries"].toArray();
        entries_.clear();
        entries_.reserve(entriesArray.size());
        
        for (const auto& value : entriesArray) {
            if (value.isObject()) {
                QJsonDocument entryDoc(value.toObject());
                entries_.push_back(VaultEntry::fromJson(entryDoc.toJson().toStdString()));
            }
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool SecureVault::saveVaultFile() {
    if (!is_open_ || vault_path_.empty()) {
        return false;
    }
    
    try {
        QJsonObject root;
        
        // Save vault metadata
        root["version"] = "1.0";
        root["salt"] = QString::fromStdString(vault_salt_);
        root["master_hash"] = QString::fromStdString(master_hash_);
        root["created_at"] = QString::fromStdString(VaultEntry::getCurrentTimestamp());
        root["device_fingerprint"] = QString::fromStdString(VaultEntry::getDeviceFingerprint());
        
        // Save entries
        QJsonArray entriesArray;
        for (const auto& entry : entries_) {
            QJsonDocument entryDoc = QJsonDocument::fromJson(QByteArray::fromStdString(entry.toJson()));
            entriesArray.append(entryDoc.object());
        }
        root["entries"] = entriesArray;
        
        QJsonDocument doc(root);
        std::string content = doc.toJson().toStdString();
        
        std::ofstream file(vault_path_);
        if (!file.good()) {
            return false;
        }
        
        file << content;
        file.close();
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string SecureVault::generateVaultMetadata() const {
    QJsonObject metadata;
    metadata["version"] = "1.0";
    metadata["created_at"] = QString::fromStdString(VaultEntry::getCurrentTimestamp());
    metadata["device_fingerprint"] = QString::fromStdString(VaultEntry::getDeviceFingerprint());
    
    QJsonDocument doc(metadata);
    return doc.toJson().toStdString();
}

bool SecureVault::parseVaultMetadata(const std::string& metadata) {
    try {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(metadata), &error);
        
        if (error.error != QJsonParseError::NoError) {
            return false;
        }
        
        // For now, just verify it's valid JSON
        // In a full implementation, you would validate version compatibility, etc.
        return doc.isObject();
        
    } catch (const std::exception&) {
        return false;
    }
}

void SecureVault::clearSensitiveData() {
    // Clear any sensitive data from memory
    for (auto& entry : entries_) {
        SecureMemory::secureZero(entry.password);
    }
}

} // namespace core
} // namespace crimson
