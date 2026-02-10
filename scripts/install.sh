#!/bin/bash

basepath=$(cd "$(dirname "$0")/../" || exit; pwd)
cd "$basepath/" || exit

PREFIX="${1:-/usr/local}"

main() {
    mkdir -p build/
    cd build/ || error_exit "Failed to cd build/"

    cmake .. -DENABLE_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" || {
        error_exit "Failed to cmake."
    }

    cmake --build . --config Release -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)" || {
        error_exit "Failed to build."
    }

    cmake --install . || {
        error_exit "Failed to install. Try: sudo sh $0 $PREFIX"
    }
}

error_exit() {
    echo
    echo "Error: $*"
    echo "----------------------------------------------------------------------"
    echo
    exit 1
}

main "$@"
exit 0
