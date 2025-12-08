# Crimson Lock

<div align="center">
  <img src="Crimson.png" alt="Crimson Lock" width="400">
  <p><em>Secure Password Manager with Hardware-based Encryption</em></p>
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
  [![Qt5](https://img.shields.io/badge/Qt-5.12+-green.svg)](https://www.qt.io/)
  [![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey.svg)](https://github.com/bacpan/Crimson-Lock)
  
</div>

A desktop application for secure password management using hardware-based randomness and military-grade encryption.



## Overview

Crimson Lock is a C++ Qt desktop application that provides:

- **Hardware-based password generation** using true random number generators
- **AES-256 encryption** with GPG for vault storage
- **Secure memory management** with locked memory and explicit data wiping
- **Auto-lock functionality** for enhanced security
- **Offline operation** - no internet connection required
- **Cross-platform support** - Linux, Windows, macOS

## Status

**COMPLETED - Fully functional application**

Core features implemented:

### Features
- C++ Qt5 desktop application
- Hardware-based password generation using `std::random_device`
- Secure memory management with memory locking and wiping
- Vault management (create, open, lock)
- Entry creation with auto-generated credentials
- Secure entry viewing with timed password display
- Clipboard management with auto-clear
- Auto-lock functionality
- JSON-based encrypted vault storage
- Cross-platform build system (CMake)

### Security
- **Qt5** framework for compatibility
- **Hardware RNG** verification and usage
- **Memory protection** with secure wiping
- **Auto-generated credentials** with customizable length and complexity

### Note
For production use, install enhanced crypto libraries:

```bash
# Install production crypto libraries (optional)
sudo apt install libargon2-dev libgpgme-dev

# Rebuild with enhanced crypto support
mkdir build && cd build
cmake .. && make
```

## Quick Start

### One-Command Setup

```bash
./run.sh
```

This script will:
1. Auto-detect your platform (Linux/macOS/Windows)
2. Check dependencies and guide installation if needed
3. Build the application with optimal settings
4. Create desktop shortcut with custom icon
5. Launch the application for immediate use

## Requirements

**⚠️ Qt5 REQUIRED: Application designed for Qt5 only. May not work with Qt6.**

### Linux
- Ubuntu 18.04+, Debian 9+, Fedora 30+
- Qt5 development libraries (`qtbase5-dev`)
- CMake 3.16+, GCC 8+, Make

### Windows  
- Windows 10+ (64-bit recommended)
- Visual Studio 2017+ or MinGW-w64
- Qt5.12+, CMake 3.16+

### macOS
- macOS 10.14+ (Mojave or newer)
- Xcode Command Line Tools
- Qt5 (`brew install qt@5`)
- CMake 3.16+ (`brew install cmake`)

### Core Dependencies
- **Qt5** - GUI framework (Qt6 not supported)
- **CMake 3.16+** - Build system  
- **C++17 compiler** - Modern C++ support

### Optional Libraries
- **libgpgme** - Enhanced GPG encryption
- **libargon2** - Production key derivation

## Build Process

### Linux (Ubuntu/Debian)
```bash
sudo apt install qtbase5-dev qt5-qmake cmake build-essential
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows
```batch
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
Release\Crimson-Lock.exe
```

### macOS
```bash
brew install qt@5 cmake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Windows Deployment

**⚠️ Qt5 REQUIRED: Ensure you have Qt5 installed. Qt6 is not supported.**

### Comprehensive Windows Deployment
```batch
deploy_windows.bat
```

**Interactive deployment with 3 options:**
1. **Portable Package** - No admin required, runs from any folder
2. **System Installation** - Requires admin, integrates with Windows
3. **Both** - Creates portable package then installs to system

**Features:**
- Automatic Qt DLL detection and bundling
- Visual C++ Redistributable installation
- Start Menu and Desktop shortcuts
- File associations (.crimson files)
- System PATH integration
- Comprehensive uninstaller
- ZIP package creation for distribution

## Desktop Integration

Platform-specific shortcuts:
- **Linux**: Desktop entry in application menu
- **Windows**: Desktop shortcut (.lnk)
- **macOS**: Native App Bundle (.app)

All shortcuts include a custom Crimson-themed icon.

## Security Features

- Hardware true random number generator (TRNG) for password generation
- Secure memory locking to prevent swapping
- Explicit memory wiping of sensitive data
- Clipboard auto-clear functionality
- Vault auto-lock after inactivity
- AES-256 encryption for vault storage
- Master password protection

## Security Architecture

```
┌─────────────────────────────┐
│         Qt UI Layer         │
│   - MainWindow              │
│   - VaultCreationDialog     │
│   - VaultViewDialog         │
└────────────┬────────────────┘
             │
┌────────────▼────────────────┐
│      Vault Security Core     │
│  - SecureVault              │
│  - CryptoManager            │
│  - PasswordGenerator (TRNG) │
│  - SecureMemory Manager     │
└────────────┬────────────────┘
             │
┌────────────▼────────────────┐
│       Encrypted Storage     │
│       vault.gpg             │
│   (AES-256 + Secure Hash)   │
└─────────────────────────────┘
```

## Security Considerations

### What This Protects Against
- Local theft of vault file  
- Offline brute-force attacks  
- Memory scraping attacks  
- Swap file password leaks  
- Clipboard harvesting  
- Weak password generation  
- Random number prediction  

### Limitations
- Cannot protect against fully compromised OS with rootkit malware  
- Cannot recover data if master password is lost  
- Social engineering attacks on the user  

## Project Structure
```
Crimson-Lock/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── core/                 # Security core components
│   │   ├── SecureVault.cpp
│   │   ├── CryptoManager.cpp
│   │   ├── PasswordGenerator.cpp
│   │   └── SecureMemory.cpp
│   └── ui/                   # Qt user interface
│       ├── MainWindow.cpp
│       ├── VaultCreationDialog.cpp
│       └── VaultViewDialog.cpp
├── include/                  # Header files
├── assets/                   # Application assets (icons)
├── CMakeLists.txt           # Build configuration
├── run.sh                   # Universal build script
└── deploy_windows.bat       # Comprehensive Windows deployment
```

## Development

**Qt5 REQUIRED: This application only works with Qt5. Qt6 compatibility is not available.**

### Building for Development
```bash
# Debug build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details on:

-  **Security-first development practices**
-  **Code style and standards**
-  **Testing requirements**
-  **Pull request process**

**Security Note**: Please report security vulnerabilities privately, not through public issues.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a detailed history of changes and releases.

---

<div align="center">
  <p><strong>⚠️ Security Warning</strong></p>
  <p>This is cryptographic software. While it implements industry-standard security practices, no software is 100% secure. Use at your own risk and maintain proper backup procedures for critical data.</p>
  <p><em>Made with by bacpan</em></p>
</div>
