#!/bin/bash

basepath=$(cd "$(dirname "$0")/../" || exit; pwd)
cd "$basepath/" || exit

main() {
    mkdir -p build/ || {
       error_exit "Failed to mkdir build/"
    }

    cd build/ || {
       error_exit "Failed to cd build/"
    }

    cmake .. -DENABLE_UNIT_TESTS=ON || {
        error_exit "Failed to cmake."
    }

    cmake --build . -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)" || {
        error_exit "Failed to build."
    }

    ctest --output-on-failure || {
        error_exit "Failed to test."
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
