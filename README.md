English | [简体中文](/README-zh-CN.md)


# Darabonba SDK Core for C++
[![codecov](https://codecov.io/gh/aliyun/tea-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/tea-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/tea-cpp.svg?branch=master)](https://travis-ci.org/aliyun/tea-cpp)


![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)


## Installation

### Requirements

#### Compiler Requirements

- **Windows**: Visual Studio 2015 or later
- **Linux**: GCC 4.9 or later
- **macOS**: Clang (Xcode Command Line Tools)

#### Build Tools

- **CMake**: 3.13 or later (3.15+ recommended for Windows)
- **C++ Standard**: C++11 or higher

#### Dependencies

This library requires the following system libraries:

- **OpenSSL**: For encryption and network communication
- **libcurl**: For HTTP requests
- **zlib**: For compression (required by curl)
- **uuid** (Linux only): For UUID generation

**Quick Install Dependencies:**

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

### Quick Installation

#### Linux/macOS

```bash
git clone https://github.com/aliyun/tea-cpp.git
cd tea-cpp
sh scripts/install.sh
```

#### Windows

1. Run the following command to clone code from Github via git-bash:

  ```bash
  git clone https://github.com/aliyun/tea-cpp.git
  ```
2. Build Visual Studio solution
  * Change directory to source code and make directory `cmake_build`
  * Open CMake UI and
    * `Browse Source` to open source code directory.
    * `Browse build`  to open the created `cmake_build` directory
    * click `configure`
    * click `generate`, Generate VS solution

3. Build and Install C++ SDK
  * Go to the cmake_build directory and open darabonba_core.sln with Visual Studio Solutions
  * Select  `Release`
  * Check INSTALL option from Build -> Configuration Manager
  * Build->Build Solutions to build.

### Quick Build

```bash
# Clone and build
git clone https://github.com/aliyun/tea-cpp.git
cd tea-cpp
mkdir build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install .
```

**📖 For detailed build instructions, see [BUILD.md](BUILD.md) | [BUILD-zh-CN.md](BUILD-zh-CN.md)**


## Issue

[Submit Issue](https://github.com/aliyun/tea-cpp/issues/new/choose), Problems that do not meet the guidelines may close immediately.


## Release notes

Detailed changes for each version are recorded in the [Release Notes](/CHANGELOG.md).

## Related

* [OpenAPI Developer Portal][open-api]
* [Latest Release][latest-release]
* [Alibabacloud Console System][console]
* [Alibaba Cloud Home Page][aliyun]

## License

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://next.api.aliyun.com
[latest-release]: https://github.com/aliyun/tea-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
