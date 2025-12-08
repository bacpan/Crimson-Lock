#include "core/SecureMemory.h"
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

// Secure memory wiping function
static void secure_zero_memory(void* ptr, size_t len) {
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) {
        *p++ = 0;
    }
    // Add a memory barrier to prevent compiler optimization
    asm volatile("" ::: "memory");
}

namespace crimson {
namespace core {

// SecureBuffer implementation
SecureMemory::SecureBuffer::SecureBuffer(size_t size) 
    : data_(nullptr), size_(size), locked_(false) {
    
    if (size == 0) {
        throw std::invalid_argument("Buffer size cannot be zero");
    }
    
    // Allocate memory
    data_ = std::malloc(size);
    if (!data_) {
        throw std::bad_alloc();
    }
    
    // Try to lock memory to prevent swapping
    locked_ = SecureMemory::lockMemory(data_, size);
    
    // Clear the memory
    std::memset(data_, 0, size);
}

SecureMemory::SecureBuffer::~SecureBuffer() {
    cleanup();
}

SecureMemory::SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept
    : data_(other.data_), size_(other.size_), locked_(other.locked_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.locked_ = false;
}

SecureMemory::SecureBuffer& SecureMemory::SecureBuffer::operator=(SecureBuffer&& other) noexcept {
    if (this != &other) {
        cleanup();
        data_ = other.data_;
        size_ = other.size_;
        locked_ = other.locked_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.locked_ = false;
    }
    return *this;
}

void SecureMemory::SecureBuffer::cleanup() {
    if (data_) {
        // Securely zero the memory
        SecureMemory::secureZero(data_, size_);
        
        // Unlock memory if it was locked
        if (locked_) {
            SecureMemory::unlockMemory(data_, size_);
        }
        
        // Free the memory
        std::free(data_);
        data_ = nullptr;
    }
    size_ = 0;
    locked_ = false;
}

// SecureMemory static methods
bool SecureMemory::lockMemory(void* addr, size_t len) {
    if (!addr || len == 0) {
        return false;
    }
    
#ifdef _WIN32
    return VirtualLock(addr, len) != 0;
#else
    return mlock(addr, len) == 0;
#endif
}

bool SecureMemory::unlockMemory(void* addr, size_t len) {
    if (!addr || len == 0) {
        return false;
    }
    
#ifdef _WIN32
    return VirtualUnlock(addr, len) != 0;
#else
    return munlock(addr, len) == 0;
#endif
}

void SecureMemory::secureZero(void* ptr, size_t len) {
    if (!ptr || len == 0) {
        return;
    }
    
    secure_zero_memory(ptr, len);
}

void SecureMemory::secureZero(std::string& str) {
    if (!str.empty()) {
        secureZero(const_cast<char*>(str.data()), str.size());
        str.clear();
    }
}

std::unique_ptr<SecureMemory::SecureBuffer> SecureMemory::createBuffer(size_t size) {
    return std::make_unique<SecureBuffer>(size);
}

} // namespace core
} // namespace crimson
