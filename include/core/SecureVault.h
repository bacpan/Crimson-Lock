#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "VaultEntry.h"
#include "CryptoManager.h"
#include "PasswordGenerator.h"
#include "SecureMemory.h"

namespace crimson {
namespace core {

/**
 * @brief Main secure vault management class
 * 
 * Handles vault creation, loading, saving, and entry management.
 * Provides the primary interface for all vault operations.
 */
class SecureVault {
public:
    SecureVault();
    ~SecureVault();
    
    /**
     * @brief Create a new vault with master password
     * @param masterPassword Master password for the vault
     * @param vaultPath Path where vault file will be stored
     * @return true if vault created successfully
     */
    bool createVault(const std::string& masterPassword, 
                     const std::string& vaultPath = "vault.gpg");
    
    /**
     * @brief Open an existing vault
     * @param masterPassword Master password
     * @param vaultPath Path to vault file
     * @return true if vault opened successfully
     */
    bool openVault(const std::string& masterPassword,
                   const std::string& vaultPath = "vault.gpg");
    
    /**
     * @brief Close and lock the vault
     */
    void closeVault();
    
    /**
     * @brief Check if vault is currently open
     */
    bool isOpen() const { return is_open_; }
    
    /**
     * @brief Create a new vault entry
     * @param label Human-readable label for the entry
     * @return Created entry with generated credentials
     */
    VaultEntry createEntry(const std::string& label);
    
    /**
     * @brief Save an entry to the vault
     * @param entry Entry to save
     * @return true if saved successfully
     */
    bool saveEntry(const VaultEntry& entry);
    
    /**
     * @brief Get all entry labels (for listing)
     * @return Vector of entry labels with IDs
     */
    std::vector<std::pair<std::string, std::string>> getEntryLabels() const;
    
    /**
     * @brief Get entry by ID (without password)
     * @param entryId Entry ID
     * @return Entry with encrypted password
     */
    VaultEntry getEntry(const std::string& entryId) const;
    
    /**
     * @brief Get decrypted password for entry
     * @param entryId Entry ID
     * @return Decrypted password
     */
    std::string getPassword(const std::string& entryId);
    
    /**
     * @brief Delete an entry
     * @param entryId Entry ID to delete
     * @return true if deleted successfully
     */
    bool deleteEntry(const std::string& entryId);
    
    /**
     * @brief Auto-lock after inactivity
     * @param timeoutSeconds Timeout in seconds (default: 60)
     */
    void setAutoLockTimeout(int timeoutSeconds = 60);
    
    /**
     * @brief Check if auto-lock should trigger
     * @return true if vault should be locked
     */
    bool shouldAutoLock() const;
    
    /**
     * @brief Update last activity timestamp
     */
    void updateActivity();
    
    /**
     * @brief Verify vault integrity
     * @return true if vault is not tampered
     */
    bool verifyIntegrity() const;
    
    /**
     * @brief Get vault statistics
     */
    struct VaultStats {
        size_t entryCount;
        std::string createdAt;
        std::string lastModified;
        std::string deviceFingerprint;
    };
    
    VaultStats getStats() const;

private:
    std::unique_ptr<CryptoManager> crypto_manager_;
    std::unique_ptr<PasswordGenerator> password_generator_;
    std::unique_ptr<SecureMemory::SecureBuffer> vault_key_;
    
    std::vector<VaultEntry> entries_;
    std::string vault_path_;
    std::string vault_salt_;
    std::string master_hash_;
    bool is_open_;
    
    // Auto-lock functionality
    std::chrono::steady_clock::time_point last_activity_;
    int auto_lock_timeout_;
    
    /**
     * @brief Load vault from file
     */
    bool loadVaultFile();
    
    /**
     * @brief Save vault to file
     */
    bool saveVaultFile();
    
    /**
     * @brief Generate vault metadata
     */
    std::string generateVaultMetadata() const;
    
    /**
     * @brief Parse vault metadata
     */
    bool parseVaultMetadata(const std::string& metadata);
    
    /**
     * @brief Clear sensitive data from memory
     */
    void clearSensitiveData();
};

} // namespace core
} // namespace crimson
