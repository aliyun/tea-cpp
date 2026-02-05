# Darabonba SDK Core C++ 构建指南

本文档提供了从源码构建 Darabonba SDK Core C++ 库的详细说明。

## 目录

- [环境要求](#环境要求)
- [依赖库](#依赖库)
- [构建配置选项](#构建配置选项)
- [构建动态库](#构建动态库)
- [构建静态库](#构建静态库)
- [指定依赖库位置](#指定依赖库位置)
- [高级构建示例](#高级构建示例)
- [平台特定说明](#平台特定说明)
- [故障排除](#故障排除)

## 环境要求

### 编译器要求

- **Windows**: Visual Studio 2015 或以上版本
- **Linux**: GCC 4.9 或以上版本
- **macOS**: Clang (Xcode Command Line Tools)

### 构建工具

- **CMake**: 3.13 或以上版本（Windows 推荐 3.15+）
- **C++ 标准**: C++11 或更高版本

### 系统要求

- **内存**: 建议 4GB 或以上
- **磁盘空间**: 建议至少 500MB 可用空间

## 依赖库

本库需要安装以下系统库：

- **OpenSSL**: 用于加密和网络通信
- **libcurl**: 用于 HTTP 请求
- **zlib**: 用于压缩（curl 依赖）
- **uuid** (仅 Linux): 用于生成 UUID

### 快速安装依赖（动态库构建）

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

### 静态库构建的额外依赖（Linux）

在 Linux 上构建静态库时，系统的静态 libcurl 可能需要额外的依赖。Ubuntu/Debian 的 libcurl 编译时启用了许多可选功能：

```bash
# Ubuntu/Debian - 静态构建额外依赖
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

**注意**：仅在以下情况需要这些依赖：
1. 使用 `-DBUILD_SHARED_LIBS=OFF` 构建静态库
2. 系统上的静态 libcurl（`libcurl.a`）编译时启用了这些功能

如果遇到类似 `undefined reference to 'nghttp2_*'` 或 `'gss_*'` 的链接错误，请安装上述对应的开发包。

## 构建配置选项

| 选项 | 默认值 | 说明 | 示例 |
|------|--------|------|------|
| `BUILD_SHARED_LIBS` | ON | 构建共享库而非静态库 | `-DBUILD_SHARED_LIBS=OFF` |
| `ENABLE_UNIT_TESTS` | OFF | 启用单元测试编译 | `-DENABLE_UNIT_TESTS=ON` |
| `CMAKE_CXX_STANDARD` | 11 | C++ 标准版本 (11/14/17/20/23) | `-DCMAKE_CXX_STANDARD=20` |
| `CMAKE_BUILD_TYPE` | - | 构建类型 (Debug/Release/RelWithDebInfo/MinSizeRel) | `-DCMAKE_BUILD_TYPE=Release` |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | 安装目录 | `-DCMAKE_INSTALL_PREFIX=/opt/alibabacloud` |

## 构建动态库

动态库是默认构建方式，适合开发和调试。

```bash
# 克隆仓库
git clone https://github.com/aliyun/tea-cpp.git
cd tea-cpp

# 创建构建目录
mkdir build && cd build

# 配置动态库构建
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_CXX_STANDARD=17

# 构建
cmake --build . --config Release --parallel 8

# 安装（Linux/macOS 可能需要 sudo）
cmake --install .
```

## 构建静态库

静态库推荐用于生产环境，因为没有运行时依赖。

```bash
mkdir build && cd build

# 配置静态库构建
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_CXX_STANDARD=17

# 构建
cmake --build . --config Release --parallel 8

# 安装
cmake --install .
```

## 指定依赖库位置

### 方法1: 使用 CMAKE_PREFIX_PATH

```bash
# 单个路径
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH="/usr/local" \
  -DCMAKE_BUILD_TYPE=Release

# 多个路径
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH="/usr/local;/opt/openssl;/opt/curl" \
  -DCMAKE_BUILD_TYPE=Release
```

### 方法2: 使用包特定变量

```bash
cmake -S .. -B . \
  -DOPENSSL_ROOT_DIR="/opt/openssl" \
  -DCURL_ROOT="/opt/curl" \
  -DCMAKE_BUILD_TYPE=Release
```

### 方法3: 使用 vcpkg (Windows)

```bash
# 首先通过 vcpkg 安装依赖
vcpkg install openssl curl zlib

# 使用 vcpkg 工具链配置
cmake -S .. -B . \
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DCMAKE_BUILD_TYPE=Release
```

### 方法4: 使用 pkg-config

```bash
# 为自定义位置设置 PKG_CONFIG_PATH
export PKG_CONFIG_PATH="/opt/openssl/lib/pkgconfig:/opt/curl/lib/pkgconfig:$PKG_CONFIG_PATH"
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
```

## 高级构建示例

### 生产发布构建（静态，优化）

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

### 开发构建（共享，带测试）

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

### 跨平台构建与自定义依赖

```bash
# Linux/macOS 使用自定义 OpenSSL 和 curl
mkdir build && cd build
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/usr/local/ssl;/usr/local/curl" \
  -DOPENSSL_USE_STATIC_LIBS=ON \
  -DCURL_STATICLIB=ON \
  -DBUILD_SHARED_LIBS=OFF

# Windows 使用 vcpkg
cmake -S .. -B . \
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-windows-static \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF
```

## 平台特定说明

### Windows (Visual Studio)

```bash
# 生成 Visual Studio 解决方案
cmake -S .. -B . -G "Visual Studio 16 2019" -A x64

# 使用 MSBuild 构建
cmake --build . --config Release --parallel 8

# 或在 Visual Studio 中打开 .sln 文件
```

### macOS (Xcode)

```bash
# 生成 Xcode 项目
cmake -S .. -B . -G Xcode

# 使用 xcodebuild 构建
cmake --build . --config Release

# Or open the .xcodeproj file in Xcode
```

### Linux (Make)

```bash
# 生成 Makefiles（默认）
cmake -S .. -B . -G "Unix Makefiles"

# 使用 make 构建
make -j8

# 或使用 cmake --build
cmake --build . --parallel 8
```

## 故障排除

### 找不到依赖

```bash
# 检查 CMake 在寻找什么
cmake -S .. -B . --debug-find

# 手动指定库路径
cmake -S .. -B . \
  -DOPENSSL_INCLUDE_DIR="/opt/openssl/include" \
  -DOPENSSL_SSL_LIBRARY="/opt/openssl/lib/libssl.a" \
  -DOPENSSL_CRYPTO_LIBRARY="/opt/openssl/lib/libcrypto.a"
```

### 静态链接问题

```bash
# 强制静态链接系统库
cmake -S .. -B . \
  -DBUILD_SHARED_LIBS=OFF \
  -DOPENSSL_USE_STATIC_LIBS=ON \
  -DCURL_STATICLIB=ON \
  -DCMAKE_FIND_LIBRARY_SUFFIXES=".a"
```

### CMake 版本问题

```bash
# 检查 CMake 版本
cmake --version

# 在 Ubuntu/Debian 上更新 CMake
sudo apt-get update
sudo apt-get install cmake

# 在 macOS 上更新 CMake
brew upgrade cmake

# 在 Windows 上更新 CMake
# 从 https://cmake.org/download/ 下载
```

### 编译器问题

```bash
# 显式指定编译器
cmake -S .. -B . \
  -DCMAKE_C_COMPILER=gcc-9 \
  -DCMAKE_CXX_COMPILER=g++-9

# 使用 Clang
cmake -S .. -B . \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
```

### 构建性能

```bash
# 使用多核编译
cmake --build . --parallel $(nproc)  # Linux
cmake --build . --parallel $(sysctl -n hw.ncpu)  # macOS
cmake --build . --parallel %NUMBER_OF_PROCESSORS%  # Windows

# 使用 Ninja 获得更快的构建速度
cmake -S .. -B . -G Ninja
ninja
```
