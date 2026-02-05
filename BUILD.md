# Build Guide for Darabonba SDK Core C++

This document provides detailed instructions for building the Darabonba SDK Core C++ library from source.

## Table of Contents

- [Requirements](#requirements)
- [Dependencies](#dependencies)
- [Build Configuration Options](#build-configuration-options)
- [Building Dynamic Libraries](#building-dynamic-libraries)
- [Building Static Libraries](#building-static-libraries)
- [Specifying Dependency Locations](#specifying-dependency-locations)
- [Advanced Build Examples](#advanced-build-examples)
- [Platform-Specific Notes](#platform-specific-notes)
- [Troubleshooting](#troubleshooting)

## Requirements

### Compiler Requirements

- **Windows**: Visual Studio 2015 or later
- **Linux**: GCC 4.9 or later
- **macOS**: Clang (Xcode Command Line Tools)

### Build Tools

- **CMake**: 3.13 or later (3.15+ recommended for Windows)
- **C++ Standard**: C++11 or higher

### System Requirements

- **Memory**: 4GB or more recommended
- **Disk Space**: At least 500MB available space

## Dependencies

This library requires the following system libraries:

- **OpenSSL**: For encryption and network communication
- **libcurl**: For HTTP requests
- **zlib**: For compression (required by curl)
- **uuid** (Linux only): For UUID generation

### Quick Install Dependencies (Dynamic Build)

```bash
# Ubuntu/Debian
sudo apt-get install -y libssl-dev libcurl4-openssl-dev zlib1g-dev uuid-dev

# CentOS/RHEL
sudo yum install -y openssl-devel libcurl-devel zlib-devel libuuid-devel

# macOS (Homebrew)
brew install openssl curl

# Windows (vcpkg)
vcpkg install openssl curl zlib
```

### Additional Dependencies for Static Build (Linux)

When building static libraries on Linux, the system's static libcurl may require additional dependencies. Ubuntu/Debian's libcurl is built with many optional features enabled:

```bash
# Ubuntu/Debian - Additional static build dependencies
sudo apt-get install -y \
  libnghttp2-dev \
  libssh-dev \
  libpsl-dev \
  libkrb5-dev \
  libldap2-dev \
  libidn2-dev \
  libbrotli-dev \
  libzstd-dev
```

**Note**: These are only required when:
1. Building with `-DBUILD_SHARED_LIBS=OFF`
2. Your system has a static libcurl (`libcurl.a`) that was built with these features enabled

If you encounter linker errors like `undefined reference to 'nghttp2_*'` or `'gss_*'`, install the corresponding development packages above.

## Build Configuration Options

| Option | Default | Description | Example |
|--------|---------|-------------|---------|
| `BUILD_SHARED_LIBS` | ON | Build shared libraries instead of static | `-DBUILD_SHARED_LIBS=OFF` |
| `ENABLE_UNIT_TESTS` | OFF | Enable unit tests compilation | `-DENABLE_UNIT_TESTS=ON` |
| `CMAKE_CXX_STANDARD` | 11 | C++ standard version (11/14/17/20/23) | `-DCMAKE_CXX_STANDARD=20` |
| `CMAKE_BUILD_TYPE` | - | Build type (Debug/Release/RelWithDebInfo/MinSizeRel) | `-DCMAKE_BUILD_TYPE=Release` |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory | `-DCMAKE_INSTALL_PREFIX=/opt/alibabacloud` |

## Building Dynamic Libraries

Dynamic libraries are built by default and are suitable for development and debugging.

```bash
# Clone the repository
git clone https://github.com/aliyun/tea-cpp.git
cd tea-cpp

# Create build directory
mkdir build && cd build

# Configure for dynamic libraries
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_CXX_STANDARD=17

# Build
cmake --build . --config Release --parallel 8

# Install (may require sudo on Linux/macOS)
cmake --install .
```

## Building Static Libraries

Static libraries are recommended for production environments as they have no runtime dependencies.

```bash
mkdir build && cd build

# Configure for static libraries
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_CXX_STANDARD=17

# Build
cmake --build . --config Release --parallel 8

# Install
cmake --install .
```

## Specifying Dependency Locations

### Method 1: Using CMAKE_PREFIX_PATH

```bash
# Single path
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH="/usr/local" \
  -DCMAKE_BUILD_TYPE=Release

# Multiple paths
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH="/usr/local;/opt/openssl;/opt/curl" \
  -DCMAKE_BUILD_TYPE=Release
```

### Method 2: Using Package-Specific Variables

```bash
cmake -S .. -B . \
  -DOPENSSL_ROOT_DIR="/opt/openssl" \
  -DCURL_ROOT="/opt/curl" \
  -DCMAKE_BUILD_TYPE=Release
```

### Method 3: Using vcpkg (Windows)

```bash
# Install dependencies via vcpkg first
vcpkg install openssl curl zlib

# Configure with vcpkg toolchain
cmake -S .. -B . \
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DCMAKE_BUILD_TYPE=Release
```

### Method 4: Using pkg-config

```bash
# Set PKG_CONFIG_PATH for custom locations
export PKG_CONFIG_PATH="/opt/openssl/lib/pkgconfig:/opt/curl/lib/pkgconfig:$PKG_CONFIG_PATH"
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
```

## Advanced Build Examples

### Production Release Build (Static, Optimized)

```bash
mkdir build-release && cd build-release
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_INSTALL_PREFIX="/opt/alibabacloud"
cmake --build . --config Release --parallel 8
cmake --install .
```

### Development Build (Shared, with Tests)

```bash
mkdir build-dev && cd build-dev
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=ON \
  -DENABLE_UNIT_TESTS=ON \
  -DCMAKE_CXX_STANDARD=20
cmake --build . --config Debug --parallel 8
ctest --output-on-failure
```

### Cross-Platform Build with Custom Dependencies

```bash
# Linux/macOS with custom OpenSSL and curl
mkdir build && cd build
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/usr/local/ssl;/usr/local/curl" \
  -DOPENSSL_USE_STATIC_LIBS=ON \
  -DCURL_STATICLIB=ON \
  -DBUILD_SHARED_LIBS=OFF

# Windows with vcpkg
cmake -S .. -B . \
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-windows-static \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF
```

## Platform-Specific Notes

### Windows (Visual Studio)

```bash
# Generate Visual Studio solution
cmake -S .. -B . -G "Visual Studio 16 2019" -A x64

# Build with MSBuild
cmake --build . --config Release --parallel 8

# Or open the .sln file in Visual Studio
```

### macOS (Xcode)

```bash
# Generate Xcode project
cmake -S .. -B . -G Xcode

# Build with xcodebuild
cmake --build . --config Release

# Or open the .xcodeproj file in Xcode
```

### Linux (Make)

```bash
# Generate Makefiles (default)
cmake -S .. -B . -G "Unix Makefiles"

# Build with make
make -j8

# Or use cmake --build
cmake --build . --parallel 8
```

## Troubleshooting

### Dependency Not Found

```bash
# Check what CMake is looking for
cmake -S .. -B . --debug-find

# Manually specify library paths
cmake -S .. -B . \
  -DOPENSSL_INCLUDE_DIR="/opt/openssl/include" \
  -DOPENSSL_SSL_LIBRARY="/opt/openssl/lib/libssl.a" \
  -DOPENSSL_CRYPTO_LIBRARY="/opt/openssl/lib/libcrypto.a"
```

### Static Linking Issues

```bash
# Force static linking of system libraries
cmake -S .. -B . \
  -DBUILD_SHARED_LIBS=OFF \
  -DOPENSSL_USE_STATIC_LIBS=ON \
  -DCURL_STATICLIB=ON \
  -DCMAKE_FIND_LIBRARY_SUFFIXES=".a"
```

### CMake Version Issues

```bash
# Check CMake version
cmake --version

# Update CMake on Ubuntu/Debian
sudo apt-get update
sudo apt-get install cmake

# Update CMake on macOS
brew upgrade cmake

# Update CMake on Windows
# Download from https://cmake.org/download/
```

### Compiler Issues

```bash
# Specify compiler explicitly
cmake -S .. -B . \
  -DCMAKE_C_COMPILER=gcc-9 \
  -DCMAKE_CXX_COMPILER=g++-9

# For Clang
cmake -S .. -B . \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
```

### Build Performance

```bash
# Use multiple cores for compilation
cmake --build . --parallel $(nproc)  # Linux
cmake --build . --parallel $(sysctl -n hw.ncpu)  # macOS
cmake --build . --parallel %NUMBER_OF_PROCESSORS%  # Windows

# Use Ninja for faster builds
cmake -S .. -B . -G Ninja
ninja
```
