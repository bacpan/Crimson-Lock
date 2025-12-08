cmake_minimum_required(VERSION 3.16)

project(CrimsonLock 
    VERSION 1.0.0 
    DESCRIPTION "Secure Password Manager with Hardware-based Encryption"
    HOMEPAGE_URL "https://github.com/bacpan/Crimson-Lock"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find required Qt5 components
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Qt5 setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/VaultCreationDialog.cpp
    src/ui/VaultViewDialog.cpp
    src/core/SecureVault.cpp
    src/core/CryptoManager.cpp
    src/core/PasswordGenerator.cpp
    src/core/SecureMemory.cpp
    src/core/VaultEntry.cpp
)

# Header files
set(HEADERS
    include/ui/MainWindow.h
    include/ui/VaultCreationDialog.h
    include/ui/VaultViewDialog.h
    include/core/SecureVault.h
    include/core/CryptoManager.h
    include/core/PasswordGenerator.h
    include/core/SecureMemory.h
    include/core/VaultEntry.h
)

# Create executable
add_executable(CrimsonLock ${SOURCES} ${HEADERS})

# Link Qt5 libraries
target_link_libraries(CrimsonLock Qt5::Core Qt5::Widgets)

# Find and link cryptographic libraries (optional for initial build)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBGPGME gpgme)
    pkg_check_modules(LIBARGON2 libargon2)
    
    if(LIBGPGME_FOUND AND LIBARGON2_FOUND)
        target_link_libraries(CrimsonLock ${LIBGPGME_LIBRARIES} ${LIBARGON2_LIBRARIES})
        target_include_directories(CrimsonLock PRIVATE ${LIBGPGME_INCLUDE_DIRS} ${LIBARGON2_INCLUDE_DIRS})
        target_compile_definitions(CrimsonLock PRIVATE HAVE_CRYPTO_LIBS)
        message(STATUS "Cryptographic libraries found - building with full security features")
    else()
        message(WARNING "Cryptographic libraries not found. Building with simplified crypto (NOT FOR PRODUCTION USE)")
        message(WARNING "Please install libgpgme and libargon2 for production deployment")
    endif()
endif()

# Compiler flags for security and optimization
target_compile_options(CrimsonLock PRIVATE 
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-fstack-protector-strong>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-D_FORTIFY_SOURCE=2>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-fPIC>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/guard:cf>
)

# Linker flags for security (Linux/Unix only)
if(UNIX AND NOT APPLE)
    target_link_options(CrimsonLock PRIVATE
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,noexecstack
    )
endif()

# Set version info for Windows
if(WIN32)
    target_compile_definitions(CrimsonLock PRIVATE 
        APP_VERSION="${PROJECT_VERSION}"
        APP_NAME="${PROJECT_NAME}"
    )
endif()

# Install target
install(TARGETS CrimsonLock
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install assets
install(DIRECTORY assets/ 
    DESTINATION share/crimsonlock/assets
    OPTIONAL
)

# Install documentation
install(FILES README.md LICENSE INSTALL.md
    DESTINATION share/doc/crimsonlock
    OPTIONAL
)
