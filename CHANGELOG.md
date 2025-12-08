# Changelog

All notable changes to Crimson Lock will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01-08

### Added
- Initial release of Crimson Lock Password Manager
- Hardware-based true random password generation using `std::random_device`
- Secure vault creation and management with master password protection
- Entry management system with auto-generated credentials
- Cross-platform support (Linux, macOS, Windows)
- Qt5-based desktop interface with modern styling
- Secure memory management with memory locking and explicit data wiping
- Auto-lock functionality for enhanced security
- Desktop integration with custom icons
- Comprehensive build and deployment scripts
- Universal `run.sh` script with platform detection
- Windows deployment script with portable and system installation options
- macOS app bundle creation
- Linux desktop integration with .desktop files
- Secure clipboard management with auto-clear functionality
- JSON-based encrypted vault storage
- Professional documentation and installation guides

### Security Features
- Hardware TRNG verification and enforcement
- AES-256 encryption support (with optional crypto libraries)
- Argon2id key derivation (with optional crypto libraries)
- Secure memory management with mlock and explicit_bzero
- Stack protection and fortified source compilation
- RELRO and NX stack protection on Linux
- Automatic core dump disabling
- Restrictive file permissions on vault files

### Technical Details
- C++17 codebase with modern best practices
- CMake-based cross-platform build system
- Qt5 framework for GUI (Qt6 not supported)
- Optional integration with libgpgme and libargon2
- Comprehensive error handling and validation
- RAII resource management throughout
- Defensive programming practices
- Security-focused code architecture

### Documentation
- Complete README with quick start guide
- Detailed INSTALL.md with platform-specific instructions
- Comprehensive security architecture documentation
- Build process documentation for all platforms
- Deployment guides for Windows, Linux, and macOS
- MIT License for open source distribution

### Build & Deployment
- Universal build script (`run.sh`) with dependency checking
- Windows deployment with windeployqt integration
- macOS app bundle creation with Info.plist
- Linux desktop integration with icon installation
- Portable deployment options
- System integration capabilities
- Professional packaging and distribution support

## [Unreleased]

### Planned Features
- Additional encryption backends
- Password strength analysis
- Import/export functionality
- Backup and restore capabilities
- Multi-language support
- Plugin architecture
- Advanced search and filtering
- Password history tracking
- Secure sharing capabilities
- Mobile companion apps

---

**Security Notice**: This software implements cryptographic functionality. While it follows industry best practices, no software is 100% secure. Users should maintain proper backup procedures and use at their own discretion.
