#!/bin/bash

# Crimson Lock - Universal Build & Launch Script
# Builds and runs the Crimson Lock application with proper platform handling

echo "Crimson Lock - Build & Launch Script"
echo "======================================="

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
    echo "Detected: $PLATFORM"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"  
    echo "Detected: $PLATFORM"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="Windows"
    echo "Detected: $PLATFORM"
else
    PLATFORM="Unknown"
    echo "Unknown platform: $OSTYPE"
fi

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the project root directory"
    echo "   Expected to find CMakeLists.txt in current directory"
    exit 1
fi

# Check for required dependencies
echo "Checking dependencies..."

# Platform-specific dependency checks
if [ "$PLATFORM" = "Linux" ]; then
    # Check for Qt5
    if ! command -v qmake-qt5 &> /dev/null && ! command -v qmake &> /dev/null; then
        echo "Qt5 development libraries not found!"
        echo "   Install with:"
        echo "   Ubuntu/Debian/Mint: sudo apt install qtbase5-dev qt5-qmake"
        echo "   Fedora/RHEL/CentOS:  sudo dnf install qt5-qtbase-devel"
        echo "   openSUSE/SLES:       sudo zypper install libqt5-qtbase-devel"
        echo "   Arch/Manjaro:        sudo pacman -S qt5-base"
        echo "   Alpine:              sudo apk add qt5-qtbase-dev"
        echo "   Gentoo:              sudo emerge dev-qt/qtcore:5"
        echo "   Void Linux:          sudo xbps-install -S qt5-devel"
        exit 1
    fi
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        echo "CMake not found!"
        echo "   Install with:"
        echo "   Ubuntu/Debian/Mint: sudo apt install cmake"
        echo "   Fedora/RHEL/CentOS:  sudo dnf install cmake"
        echo "   openSUSE/SLES:       sudo zypper install cmake"
        echo "   Arch/Manjaro:        sudo pacman -S cmake"
        echo "   Alpine:              sudo apk add cmake"
        echo "   Gentoo:              sudo emerge dev-util/cmake"
        echo "   Void Linux:          sudo xbps-install -S cmake"
        exit 1
    fi
    
    # Check for build tools
    if ! command -v make &> /dev/null; then
        echo "Make not found!"
        echo "   Install with:"
        echo "   Ubuntu/Debian/Mint: sudo apt install build-essential"
        echo "   Fedora/RHEL/CentOS:  sudo dnf groupinstall 'Development Tools'"
        echo "   openSUSE/SLES:       sudo zypper install -t pattern devel_basis"
        echo "   Arch/Manjaro:        sudo pacman -S base-devel"
        echo "   Alpine:              sudo apk add build-base"
        echo "   Gentoo:              Built-in with system"
        echo "   Void Linux:          sudo xbps-install -S base-devel"
        exit 1
    fi
    
elif [ "$PLATFORM" = "macOS" ]; then
    # Check for Xcode Command Line Tools
    if ! command -v clang &> /dev/null; then
        echo "Xcode Command Line Tools not found!"
        echo "   Install with: xcode-select --install"
        exit 1
    fi
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        echo "CMake not found!"
        echo "   Install with Homebrew: brew install cmake"
        exit 1
    fi
    
    # Check for Qt5
    if ! command -v qmake &> /dev/null; then
        echo "Qt5 development libraries not found!"
        echo "   Install with Homebrew: brew install qt@5"
        exit 1
    fi
    
elif [ "$PLATFORM" = "Windows" ]; then
    echo "Windows detected - this script works in Git Bash/WSL"
    echo "   For native Windows, please use Visual Studio or install:"
    echo "   - CMake: https://cmake.org/download/"
    echo "   - Qt5: https://www.qt.io/download"
    echo "   - Visual Studio Build Tools or MinGW"
    echo ""
    
    # Check for basic Windows build tools
    if command -v cl &> /dev/null; then
        echo "MSVC compiler found"
    elif command -v gcc &> /dev/null || command -v g++ &> /dev/null; then
        echo "GCC/MinGW compiler found"
    else
        echo "No compiler detected - please install Visual Studio or MinGW"
    fi
fi

echo "All dependencies found!"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
if [ "$PLATFORM" = "macOS" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Debug
elif [ "$PLATFORM" = "Windows" ]; then
    # Try to detect if we have MSVC or MinGW
    if command -v cl &> /dev/null; then
        cmake .. -DCMAKE_BUILD_TYPE=Debug
    else
        cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
    fi
else
    cmake .. -DCMAKE_BUILD_TYPE=Debug
fi

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the application
echo "Building application..."
if [ "$PLATFORM" = "macOS" ]; then
    make -j$(sysctl -n hw.ncpu)
elif [ "$PLATFORM" = "Windows" ] && command -v cl &> /dev/null; then
    cmake --build . --config Debug
else
    make -j$(nproc 2>/dev/null || echo 4)
fi

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"

cd ..

# Function to install icon and create desktop entry
install_desktop_integration() {
    if [ "$PLATFORM" = "Linux" ]; then
        echo "Installing application icon and creating desktop entry..."
        echo "   Desktop Environment: ${XDG_CURRENT_DESKTOP:-Unknown}"
        echo "   Distribution: $(lsb_release -ds 2>/dev/null || (source /etc/os-release 2>/dev/null && echo "$PRETTY_NAME") || (grep "PRETTY_NAME" /etc/os-release 2>/dev/null | cut -d'"' -f2) || (grep "^NAME" /etc/os-release 2>/dev/null | cut -d'"' -f2) || echo "Unknown Linux")"
        
        # Get absolute paths
        PROJECT_DIR="$(pwd)"
        ICON_SOURCE="$PROJECT_DIR/assets/crimson-lock-icon.png"
        EXECUTABLE_PATH="$PROJECT_DIR/build/CrimsonLock"
        
        # Validate executable exists
        if [ ! -f "$EXECUTABLE_PATH" ]; then
            echo "Error: Executable not found at $EXECUTABLE_PATH"
            echo "   Please build the project first"
            return 1
        fi
        
        # Install icon to user icon directory - support multiple sizes
        echo "Installing application icon..."
        
        # Create icon directories for multiple sizes
        for SIZE in 16x16 22x22 24x24 32x32 48x48 64x64 96x96 128x128 256x256 512x512; do
            ICON_DIR="$HOME/.local/share/icons/hicolor/$SIZE/apps"
            mkdir -p "$ICON_DIR"
        done
        
        # Install the main icon (copy to multiple sizes for compatibility)
        if [ -f "$ICON_SOURCE" ]; then
            # Install to most common sizes
            for SIZE in 48x48 64x64 128x128 256x256 512x512; do
                ICON_DIR="$HOME/.local/share/icons/hicolor/$SIZE/apps"
                cp "$ICON_SOURCE" "$ICON_DIR/crimson-lock.png"
            done
            echo "Icon installed to hicolor theme (multiple sizes)"
        else
            echo "Warning: Icon file not found at $ICON_SOURCE"
            echo "   Desktop entry will be created without custom icon"
        fi
        
        # Create desktop entry directory
        DESKTOP_DIR="$HOME/.local/share/applications"
        mkdir -p "$DESKTOP_DIR"
        DESKTOP_FILE="$DESKTOP_DIR/crimson-lock.desktop"
        
        echo "Creating desktop entry..."
        cat > "$DESKTOP_FILE" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Crimson Lock
GenericName=Password Manager
Comment=Secure Password Manager with Hardware-based Encryption
Exec="$EXECUTABLE_PATH"
Icon=crimson-lock
Terminal=false
Categories=Utility;Security;
Keywords=password;manager;security;vault;encryption;credentials;login;
StartupWMClass=Crimson-Lock
StartupNotify=true
MimeType=application/x-crimson-vault;
X-GNOME-UsesNotifications=true
EOF
        
        # Make desktop file executable
        chmod +x "$DESKTOP_FILE"
        echo "Desktop entry created at $DESKTOP_FILE"
        
        # Update desktop database (works on most Linux distributions)
        echo "Updating system databases..."
        if command -v update-desktop-database &> /dev/null; then
            update-desktop-database "$DESKTOP_DIR" 2>/dev/null && echo "Desktop database updated"
        elif [ -x "/usr/bin/update-desktop-database" ]; then
            /usr/bin/update-desktop-database "$DESKTOP_DIR" 2>/dev/null && echo "Desktop database updated"
        elif [ -x "/usr/local/bin/update-desktop-database" ]; then
            /usr/local/bin/update-desktop-database "$DESKTOP_DIR" 2>/dev/null && echo "Desktop database updated"
        else
            echo "Desktop database update tool not found (desktop entry will still work)"
        fi
        
        # Update icon cache (GTK-based desktops)
        if command -v gtk-update-icon-cache &> /dev/null; then
            gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null && echo "GTK icon cache updated"
        elif [ -x "/usr/bin/gtk-update-icon-cache" ]; then
            /usr/bin/gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null && echo "GTK icon cache updated"
        elif [ -x "/usr/local/bin/gtk-update-icon-cache" ]; then
            /usr/local/bin/gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null && echo "GTK icon cache updated"
        else
            echo "GTK icon cache tool not found (icons will still work after restart)"
        fi
        
        # Update icon cache (KDE)
        if command -v kbuildsycoca5 &> /dev/null; then
            kbuildsycoca5 &>/dev/null && echo "KDE system configuration cache updated"
        elif command -v kbuildsycoca6 &> /dev/null; then
            kbuildsycoca6 &>/dev/null && echo "KDE system configuration cache updated"
        fi
        
        # Desktop environment specific updates
        DE="${XDG_CURRENT_DESKTOP:-${DESKTOP_SESSION:-Unknown}}"
        case "$DE" in
            "GNOME"|"ubuntu:GNOME"|"GNOME-Classic"|"gnome"|"Unity")
                echo "GNOME/Unity desktop detected"
                # Force GNOME Shell to reload extensions and apps
                if command -v gdbus &> /dev/null; then
                    gdbus call --session --dest org.gnome.Shell --object-path /org/gnome/Shell --method org.gnome.Shell.Eval "Main.overview._dash._box.destroy_all_children(); Main.overview._dash._box._init();" 2>/dev/null || true
                fi
                ;;
            "KDE"|"plasma"|"kde-plasma"|"KDE5")
                echo "KDE Plasma desktop detected"
                # KDE cache updates already handled above
                ;;
            "XFCE"|"XFCE4"|"xfce"|"xfce4")
                echo "XFCE desktop detected"
                if command -v xfce4-panel &> /dev/null; then
                    xfce4-panel -r 2>/dev/null &
                fi
                ;;
            "MATE"|"mate")
                echo "MATE desktop detected"
                ;;
            "Cinnamon"|"cinnamon"|"X-Cinnamon")
                echo "Cinnamon desktop detected"
                ;;
            "LXDE"|"lxde"|"LXQT"|"lxqt")
                echo "LXDE/LXQT desktop detected"
                ;;
            "Pantheon"|"pantheon")
                echo "Pantheon (elementary) desktop detected"
                ;;
            "Budgie"|"budgie")
                echo "Budgie desktop detected"
                ;;
            "i3"|"i3wm"|"sway"|"dwm"|"awesome"|"openbox")
                echo "Tiling/Minimal window manager detected"
                ;;
            *)
                echo "Desktop environment: $DE"
                ;;
        esac
        
        # Test if desktop file is valid
        if command -v desktop-file-validate &> /dev/null || [ -x "/usr/bin/desktop-file-validate" ]; then
            if (desktop-file-validate "$DESKTOP_FILE" || /usr/bin/desktop-file-validate "$DESKTOP_FILE") 2>/dev/null; then
                echo "Desktop file validation passed"
            else
                echo "Desktop file validation failed (may still work)"
            fi
        fi
        
        # Create uninstaller script
        UNINSTALL_SCRIPT="$PROJECT_DIR/uninstall-desktop.sh"
        cat > "$UNINSTALL_SCRIPT" << 'UNINSTALL_EOF'
#!/bin/bash
echo "Removing Crimson Lock desktop integration..."
rm -f "$HOME/.local/share/applications/crimson-lock.desktop"
for SIZE in 16x16 22x22 24x24 32x32 48x48 64x64 96x96 128x128 256x256 512x512; do
    rm -f "$HOME/.local/share/icons/hicolor/$SIZE/apps/crimson-lock.png"
done

# Update desktop database
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
elif [ -x "/usr/bin/update-desktop-database" ]; then
    /usr/bin/update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
elif [ -x "/usr/local/bin/update-desktop-database" ]; then
    /usr/local/bin/update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
fi

# Update GTK icon cache  
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
elif [ -x "/usr/bin/gtk-update-icon-cache" ]; then
    /usr/bin/gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
elif [ -x "/usr/local/bin/gtk-update-icon-cache" ]; then
    /usr/local/bin/gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
fi

echo "Crimson Lock removed from desktop"
UNINSTALL_EOF
        chmod +x "$UNINSTALL_SCRIPT"
        
        echo ""
        echo "Crimson Lock is now installed and available in your applications menu!"
        echo "   • Look for 'Crimson Lock' in your application launcher"
        echo "   • Check the 'Utilities' or 'Security' category"
        echo "   • You can also search for 'password' or 'crimson'"
        echo ""
        echo "To uninstall desktop integration later, run: $UNINSTALL_SCRIPT"
        
    elif [ "$PLATFORM" = "macOS" ]; then
        echo "Creating macOS App Bundle..."
        
        APP_DIR="./Crimson-Lock.app"
        CONTENTS_DIR="$APP_DIR/Contents"
        MACOS_DIR="$CONTENTS_DIR/MacOS"
        RESOURCES_DIR="$CONTENTS_DIR/Resources"
        
        mkdir -p "$MACOS_DIR" "$RESOURCES_DIR"
        
        # Copy executable
        cp "build/CrimsonLock" "$MACOS_DIR/"
        
        # Copy icon if available
        if [ -f "assets/crimson-lock-icon.png" ]; then
            cp "assets/crimson-lock-icon.png" "$RESOURCES_DIR/crimson-lock.png"
        fi
        
        # Create Info.plist
        cat > "$CONTENTS_DIR/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>CrimsonLock</string>
    <key>CFBundleIdentifier</key>
    <string>com.github.bacpan.crimsonlock</string>
    <key>CFBundleName</key>
    <string>Crimson Lock</string>
    <key>CFBundleDisplayName</key>
    <string>Crimson Lock</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>CRLK</string>
    <key>CFBundleIconFile</key>
    <string>crimson-lock.png</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.14</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSHumanReadableCopyright</key>
    <string>Copyright © 2025 bacpan. All rights reserved.</string>
</dict>
</plist>
EOF
        
        echo "macOS App Bundle created at $APP_DIR"
        echo "You can now drag Crimson-Lock.app to your Applications folder!"
    fi
}

# Offer to create desktop shortcut/app bundle
if [ "$PLATFORM" = "Linux" ]; then
    SHORTCUT_PATH="$HOME/.local/share/applications/crimson-lock.desktop"
    echo ""
    echo "======================================"
    echo "  DESKTOP INTEGRATION"
    echo "======================================"
    if [ -f "$SHORTCUT_PATH" ]; then
        read -p "Desktop entry already exists. Reinstall/Update Crimson Lock in applications menu? [Y/n]: " -n 1 -r
    else
        read -p "Install Crimson Lock to desktop applications menu? [Y/n]: " -n 1 -r
    fi
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        install_desktop_integration
    fi
elif [ "$PLATFORM" = "macOS" ]; then
    APP_BUNDLE_PATH="./Crimson-Lock.app"
    echo ""
    echo "======================================"
    echo "  MACOS APP BUNDLE"
    echo "======================================"
    if [ -d "$APP_BUNDLE_PATH" ]; then
        read -p "App Bundle already exists. Recreate macOS App Bundle? [Y/n]: " -n 1 -r
    else
        read -p "Create macOS App Bundle? [Y/n]: " -n 1 -r
    fi
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        install_desktop_integration
    fi
fi

# Check if we should run the application
echo ""
echo "======================================"
echo "  LAUNCH APPLICATION"
echo "======================================"
read -p "Launch Crimson Lock now? [Y/n]: " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
    echo "Launching Crimson Lock..."
    echo "   (Close the application window to return to terminal)"
    echo ""
    
    cd build
    
    # Platform-specific launch with compatibility fixes
    if [ "$PLATFORM" = "Linux" ]; then
        # Handle Wayland/X11 compatibility
        if [ "$XDG_SESSION_TYPE" = "wayland" ]; then
            echo "Using XCB platform for Wayland compatibility"
            QT_QPA_PLATFORM=xcb ./CrimsonLock
        else
            ./CrimsonLock
        fi
    elif [ "$PLATFORM" = "macOS" ]; then
        # Use Cocoa platform explicitly
        QT_QPA_PLATFORM=cocoa ./CrimsonLock
    else
        ./CrimsonLock
    fi
else
    echo "Application built successfully!"
    echo ""
    echo "To run manually:"
    echo "  ./build/CrimsonLock"
    if [ "$PLATFORM" = "Linux" ] && [ "$XDG_SESSION_TYPE" = "wayland" ]; then
        echo "  or: QT_QPA_PLATFORM=xcb ./build/CrimsonLock  (for Wayland)"
    fi
fi
