#pragma once

#include <string>
#include <memory>

namespace crimson {
namespace core {

/**
 * @brief Secure memory management utilities
 * 
 * Provides functions for:
 * - Locking memory to prevent swapping
 * - Securely zeroing sensitive data
 * - RAII wrapper for secure memory regions
 */
class SecureMemory {
public:
    /**
     * @brief RAII wrapper for secure memory allocation
     */
    class SecureBuffer {
    public:
        explicit SecureBuffer(size_t size);
        ~SecureBuffer();
        
        // Non-copyable
        SecureBuffer(const SecureBuffer&) = delete;
        SecureBuffer& operator=(const SecureBuffer&) = delete;
        
        // Movable
        SecureBuffer(SecureBuffer&& other) noexcept;
        SecureBuffer& operator=(SecureBuffer&& other) noexcept;
        
        void* data() const { return data_; }
        size_t size() const { return size_; }
        
        template<typename T>
        T* as() const { return static_cast<T*>(data_); }
        
    private:
        void* data_;
        size_t size_;
        bool locked_;
        
        void cleanup();
    };
    
    /**
     * @brief Lock memory to prevent it from being swapped to disk
     */
    static bool lockMemory(void* addr, size_t len);
    
    /**
     * @brief Unlock previously locked memory
     */
    static bool unlockMemory(void* addr, size_t len);
    
    /**
     * @brief Securely zero memory
     */
    static void secureZero(void* ptr, size_t len);
    
    /**
     * @brief Securely zero a string
     */
    static void secureZero(std::string& str);
    
    /**
     * @brief Create a secure buffer
     */
    static std::unique_ptr<SecureBuffer> createBuffer(size_t size);
};

} // namespace core
} // namespace crimson
