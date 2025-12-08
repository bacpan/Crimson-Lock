#pragma once

#include <string>
#include <vector>
#include <memory>

namespace crimson {
namespace core {

/**
 * @brief Represents a single vault entry
 */
struct VaultEntry {
    std::string id;           // UUID
    std::string label;        // Human-readable label
    std::string username;     // Generated username
    std::string password;     // Encrypted password
    std::string created_at;   // ISO timestamp
    std::string device_fingerprint; // SHA256 of device ID
    
    /**
     * @brief Convert entry to JSON string
     */
    std::string toJson() const;
    
    /**
     * @brief Create entry from JSON string
     */
    static VaultEntry fromJson(const std::string& json);
    
    /**
     * @brief Generate a new UUID
     */
    static std::string generateUuid();
    
    /**
     * @brief Get current timestamp in ISO format
     */
    static std::string getCurrentTimestamp();
    
    /**
     * @brief Generate device fingerprint
     */
    static std::string getDeviceFingerprint();
};

} // namespace core
} // namespace crimson
