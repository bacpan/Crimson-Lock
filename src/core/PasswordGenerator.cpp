#include "core/PasswordGenerator.h"
#include "core/SecureMemory.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <array>

namespace crimson {
namespace core {

// Character sets for password generation
const std::string PasswordGenerator::LOWERCASE_CHARS = "abcdefghijklmnopqrstuvwxyz";
const std::string PasswordGenerator::UPPERCASE_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string PasswordGenerator::DIGIT_CHARS = "0123456789";
const std::string PasswordGenerator::SYMBOL_CHARS = "!@#$%^&*()_+-=[]{}|;:,.<>?";

PasswordGenerator::PasswordGenerator() : hardware_rng_(), secure_prng_(hardware_rng_()) {
    seedPrng();
}

PasswordGenerator::~PasswordGenerator() {
    // No sensitive data to clean up here
}

std::string PasswordGenerator::generatePassword(size_t length, bool includeSymbols) {
    if (length < 8) {
        throw std::invalid_argument("Password length must be at least 8 characters");
    }
    
    if (length > 512) {
        throw std::invalid_argument("Password length cannot exceed 512 characters");
    }
    
    // Build character set
    std::string charset = LOWERCASE_CHARS + UPPERCASE_CHARS + DIGIT_CHARS;
    if (includeSymbols) {
        charset += SYMBOL_CHARS;
    }
    
    // Generate password ensuring at least one character from each required set
    std::string password;
    password.reserve(length);
    
    // Ensure at least one character from each required set
    password += generateFromCharset(LOWERCASE_CHARS, 1);
    password += generateFromCharset(UPPERCASE_CHARS, 1);
    password += generateFromCharset(DIGIT_CHARS, 1);
    
    if (includeSymbols) {
        password += generateFromCharset(SYMBOL_CHARS, 1);
    }
    
    // Fill remaining length with random characters from full charset
    size_t remaining = length - password.length();
    password += generateFromCharset(charset, remaining);
    
    // Shuffle the password to avoid predictable patterns
    std::shuffle(password.begin(), password.end(), secure_prng_);
    
    return password;
}

std::string PasswordGenerator::generateUsername(const std::string& basePrefix) {
    std::string username = basePrefix;
    
    // Add random alphanumeric suffix
    std::string charset = LOWERCASE_CHARS + UPPERCASE_CHARS + DIGIT_CHARS;
    std::string suffix = generateFromCharset(charset, 8);
    
    username += suffix;
    
    return username;
}

void PasswordGenerator::generateRandomBytes(uint8_t* buffer, size_t size) {
    if (!buffer || size == 0) {
        throw std::invalid_argument("Invalid buffer parameters");
    }
    
    // Re-seed periodically for extra security
    static size_t bytes_generated = 0;
    bytes_generated += size;
    if (bytes_generated > 1024 * 1024) {  // Re-seed every 1MB
        seedPrng();
        bytes_generated = 0;
    }
    
    // Generate random bytes using PRNG seeded with hardware entropy
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = static_cast<uint8_t>(secure_prng_() & 0xFF);
    }
}

bool PasswordGenerator::isHardwareRngAvailable() {
    try {
        std::random_device rd;
        
        // Test entropy - hardware RNG should provide good entropy
        uint32_t test1 = rd();
        uint32_t test2 = rd();
        
        // Basic check - values should not be identical (extremely unlikely with true RNG)
        if (test1 == test2) {
            return false;
        }
        
#ifdef __linux__
        // On Linux, check if /dev/urandom is available and readable
        std::ifstream urandom("/dev/urandom", std::ios::binary);
        if (!urandom.good()) {
            return false;
        }
        
        // Test reading from urandom
        char test_byte;
        urandom.read(&test_byte, 1);
        if (!urandom.good()) {
            return false;
        }
#endif
        
        return true;
        
    } catch (...) {
        return false;
    }
}

void PasswordGenerator::seedPrng() {
    try {
        // Gather entropy from hardware RNG
        std::array<uint32_t, 8> entropy;
        for (size_t i = 0; i < entropy.size(); ++i) {
            entropy[i] = hardware_rng_();
        }
        
        // Create seed sequence from entropy
        std::seed_seq seed(entropy.begin(), entropy.end());
        
        // Seed the PRNG
        secure_prng_.seed(seed);
        
        // Discard some initial values for extra security
        secure_prng_.discard(1000);
        
    } catch (...) {
        throw std::runtime_error("Failed to seed secure PRNG with hardware entropy");
    }
}

std::string PasswordGenerator::generateFromCharset(const std::string& charset, size_t length) {
    if (charset.empty() || length == 0) {
        return "";
    }
    
    std::string result;
    result.reserve(length);
    
    std::uniform_int_distribution<size_t> dist(0, charset.length() - 1);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(secure_prng_)];
    }
    
    return result;
}

} // namespace core
} // namespace crimson
