# Crimson Lock Installation Guide

## Prerequisites

### Required Dependencies

1. **Qt 5** - GUI framework (Qt 5.12 or higher recommended)
2. **CMake** (3.16 or higher) - Build system
3. **C++17 compatible compiler** (GCC 7+, Clang 6+, MSVC 2019+)
4. **libgpgme** - GPG Made Easy library for cryptographic operations
5. **libargon2** - Argon2 password hashing library

### Installing Dependencies

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install cmake build-essential qt5-default libqt5widgets5 qtbase5-dev pkg-config
sudo apt install libgpgme-dev libargon2-dev
```

#### Fedora/RHEL:
```bash
sudo dnf install cmake gcc-c++ qt5-qtbase-devel pkgconfig
sudo dnf install gpgme-devel libargon2-devel
```

#### Arch Linux:
```bash
sudo pacman -S cmake gcc qt5-base pkgconf
sudo pacman -S gpgme argon2
```

#### Windows (using vcpkg):
```cmd
vcpkg install qt5 gpgme argon2
```

#### macOS (using Homebrew):
```bash
brew install cmake qt5 pkg-config
brew install gpgme argon2
```

## Building

1. **Clone or extract the project**
2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

4. **Build the project**:
   ```bash
   make -j$(nproc)
   ```

5. **Install (optional)**:
   ```bash
   sudo make install
   ```

## Quick Start with Desktop Integration

For the easiest setup with desktop integration, use the provided script:

```bash
./run.sh
```

This script will:
- Check all dependencies automatically
- Build the application
- Offer to install the app icon to your system
- Create a desktop entry for easy access from your applications menu
- Launch the application

## Manual Desktop Integration

### Linux Desktop Entry

To manually add Crimson Lock to your applications menu:

1. **Install the icon**:
   ```bash
   mkdir -p ~/.local/share/icons/hicolor/512x512/apps
   cp assets/crimson-lock-icon.png ~/.local/share/icons/hicolor/512x512/apps/crimson-lock.png
   ```

2. **Create desktop entry**:
   ```bash
   mkdir -p ~/.local/share/applications
   cat > ~/.local/share/applications/crimson-lock.desktop << 'EOF'
   [Desktop Entry]
   Version=1.0
   Type=Application
   Name=Crimson Lock
   Comment=Secure Password Manager with Hardware-based Encryption
   Exec=/path/to/your/build/CrimsonLock
   Icon=crimson-lock
   Terminal=false
   Categories=Utility;Security;
   Keywords=password;manager;security;vault;encryption;
   StartupWMClass=CrimsonLock
   EOF
   ```
   **Note**: Replace `/path/to/your/build/CrimsonLock` with the actual path to your executable.

3. **Update system databases**:
   ```bash
   update-desktop-database ~/.local/share/applications
   gtk-update-icon-cache -t ~/.local/share/icons/hicolor
   ```

### macOS App Bundle

For macOS, the `run.sh` script can create a proper App Bundle that you can drag to your Applications folder.

## Running

After building, you can run the application:
```bash
./CrimsonLock
```

## Security Notes

- The application requires hardware random number generation support
- All cryptographic operations use industry-standard libraries
- Data directory permissions are automatically set to owner-only access
- Core dumps are disabled for security

## Troubleshooting

### Build Issues

1. **Qt5 not found**: Ensure Qt5 is properly installed and CMake can find it
2. **Missing crypto libraries**: Install libgpgme and libargon2 development packages
3. **Compiler errors**: Ensure you have a C++17 compatible compiler

### Runtime Issues

1. **Hardware RNG not available**: Check if your system supports hardware random number generation
2. **Permission errors**: Ensure the application has write access to the user data directory
3. **Missing libraries**: Verify all runtime dependencies are installed

## Development

For development builds, you may want to enable debug mode:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

This will include debugging symbols and additional runtime checks.
