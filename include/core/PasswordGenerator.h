#pragma once

#include <string>
#include <vector>
#include <random>
#include "SecureMemory.h"

namespace crimson {
namespace core {

/**
 * @brief Hardware-based secure password generator
 * 
 * Uses hardware true random number generator for maximum entropy.
 * Generates cryptographically secure passwords and usernames.
 */
class PasswordGenerator {
public:
    PasswordGenerator();
    ~PasswordGenerator();
    
    /**
     * @brief Generate a secure random password
     * @param length Password length (default: 64)
     * @param includeSymbols Include special symbols (default: true)
     * @return Randomly generated password
     */
    std::string generatePassword(size_t length = 64, bool includeSymbols = true);
    
    /**
     * @brief Generate a secure random username
     * @param basePrefix Base prefix for username (default: "user_")
     * @return Randomly generated username
     */
    std::string generateUsername(const std::string& basePrefix = "user_");
    
    /**
     * @brief Generate random bytes using hardware RNG
     * @param buffer Buffer to fill with random data
     * @param size Number of bytes to generate
     */
    void generateRandomBytes(uint8_t* buffer, size_t size);
    
    /**
     * @brief Test hardware RNG availability
     * @return true if hardware RNG is available and working
     */
    static bool isHardwareRngAvailable();
    
private:
    std::random_device hardware_rng_;
    std::mt19937_64 secure_prng_;
    
    // Character sets for password generation
    static const std::string LOWERCASE_CHARS;
    static const std::string UPPERCASE_CHARS;
    static const std::string DIGIT_CHARS;
    static const std::string SYMBOL_CHARS;
    
    /**
     * @brief Seed the PRNG with hardware entropy
     */
    void seedPrng();
    
    /**
     * @brief Generate random string from character set
     */
    std::string generateFromCharset(const std::string& charset, size_t length);
};

} // namespace core
} // namespace crimson
