# Contributing to Crimson Lock

Thank you for your interest in contributing to Crimson Lock! This document provides guidelines for contributing to this security-focused password manager project.

## 🚨 Security First

**CRITICAL**: This is cryptographic software handling sensitive user data. All contributions must prioritize security.

### Security Review Process
- All security-related changes require thorough review
- Cryptographic implementations must follow established standards
- Memory management must prevent information leaks
- Input validation is mandatory for all external data

## 📋 Getting Started

### Prerequisites
- C++17 compatible compiler (GCC 8+, Clang 6+, MSVC 2019+)
- Qt5 development libraries (Qt6 not supported)
- CMake 3.16+
- Git for version control

### Development Setup

1. **Fork and clone the repository**:
   ```bash
   git clone https://github.com/yourusername/Crimson-Lock.git
   cd Crimson-Lock
   ```

2. **Install dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt install qtbase5-dev qt5-qmake cmake build-essential libgpgme-dev libargon2-dev
   
   # macOS
   brew install qt@5 cmake gpgme argon2
   
   # Windows (using vcpkg)
   vcpkg install qt5 gpgme argon2
   ```

3. **Build the project**:
   ```bash
   ./run.sh
   ```

## 🏗️ Development Guidelines

### Code Style

#### C++ Guidelines
- Follow C++17 best practices
- Use RAII for resource management
- Prefer smart pointers over raw pointers
- Use const-correctness throughout
- Follow secure coding patterns

#### Qt Guidelines
- Use Qt5 conventions and naming
- Implement proper signal-slot connections
- Use Qt's memory management appropriately
- Follow Qt's object lifetime patterns

#### Security Guidelines
- **Memory Management**: Always use secure memory for sensitive data
- **Random Generation**: Use hardware TRNG via `std::random_device`
- **Crypto Operations**: Implement through the security core only
- **Error Handling**: Never leak information in error messages
- **Input Validation**: Validate all external inputs
- **Resource Cleanup**: Explicitly zero sensitive data after use

### Code Organization

```
├── include/                    # Header files
│   ├── core/                  # Security core headers
│   └── ui/                    # UI headers
├── src/                       # Source files
│   ├── core/                  # Security implementation
│   └── ui/                    # UI implementation
├── tests/                     # Unit and integration tests
└── docs/                      # Additional documentation
```

## 🧪 Testing

### Test Requirements
- All new features must include tests
- Security-critical code requires comprehensive testing
- Tests must not expose sensitive data
- Memory leaks are not tolerated

### Running Tests
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON ..
make
ctest
```

### Writing Tests
- Use Qt Test framework for UI testing
- Use Google Test for core functionality (if available)
- Mock external dependencies
- Test error conditions thoroughly

## 🔒 Security Contributions

### Cryptographic Code
- Must use established algorithms (AES-256, Argon2id)
- Implement constant-time operations where applicable
- Follow timing attack prevention patterns
- Document security assumptions

### Memory Management
- Use `SecureMemory` class for sensitive data
- Call `explicit_bzero` or equivalent for cleanup
- Lock memory pages containing secrets
- Prevent core dumps in security-critical sections

### Random Number Generation
- Always use hardware TRNG via `PasswordGenerator`
- Never use predictable sources for security operations
- Validate entropy availability before use

## 📝 Pull Request Process

### Before Submitting
1. **Test thoroughly** on your target platform(s)
2. **Run security checks** and memory analysis
3. **Update documentation** for any API changes
4. **Follow commit message conventions**

### Commit Messages
Use conventional commit format:
```
type(scope): description

[optional body]

[optional footer]
```

Examples:
- `feat(core): add hardware entropy verification`
- `fix(ui): prevent password field memory leak`
- `docs(README): update build instructions`
- `security(crypto): implement constant-time comparison`

### Review Process
1. Automated checks must pass
2. Security review for sensitive changes
3. Code review by maintainers
4. Testing on multiple platforms
5. Documentation review

## 🐛 Bug Reports

### Security Issues
**DO NOT** open public issues for security vulnerabilities!
- Email security issues privately to the maintainers
- Include detailed reproduction steps
- Provide environment information
- Allow time for patch development before disclosure

### Regular Issues
Use the issue template and provide:
- Clear description of the problem
- Steps to reproduce
- Expected vs actual behavior
- Platform and version information
- Relevant logs (sanitized of sensitive data)

## 🎯 Feature Requests

### Before Requesting
1. Check existing issues and discussions
2. Consider security implications
3. Evaluate cross-platform compatibility
4. Assess maintenance burden

### Request Format
- Clear use case description
- Security considerations
- Implementation suggestions
- Alternative solutions considered

## 📚 Documentation

### Required Documentation
- API documentation for public interfaces
- Security assumptions and threat model
- Build and deployment instructions
- User guides for new features

### Documentation Tools
- Use Doxygen for code documentation
- Update README.md for user-facing changes
- Maintain CHANGELOG.md for releases

## 🌍 Community

### Code of Conduct
- Be respectful and inclusive
- Focus on technical merit
- Prioritize security and user safety
- Help others learn and contribute

### Communication
- Use GitHub Issues for bug reports
- Use GitHub Discussions for questions
- Keep security discussions private
- Be patient with review processes

## 📄 Legal

### Licensing
- All contributions must be compatible with MIT License
- You retain copyright to your contributions
- Contributions are licensed under project's MIT License

### DCO (Developer Certificate of Origin)
By submitting a pull request, you certify that:
- You wrote the code or have permission to submit it
- You understand it will be distributed under the MIT License
- You have the right to submit the contribution

## 🔄 Release Process

### Version Numbering
We follow [Semantic Versioning](https://semver.org/):
- MAJOR: Breaking changes or significant security updates
- MINOR: New features (backward compatible)
- PATCH: Bug fixes (backward compatible)

### Release Criteria
- All tests passing
- Security review completed
- Documentation updated
- Cross-platform testing completed
- CHANGELOG.md updated

---

## 🆘 Need Help?

- 📖 Check the [README.md](README.md) and [INSTALL.md](INSTALL.md)
- 💬 Use GitHub Discussions for questions
- 🐛 Report bugs via GitHub Issues
- 🔒 Email security issues privately

Thank you for helping make Crimson Lock more secure and reliable! 🔐
