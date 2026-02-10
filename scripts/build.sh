#!/bin/bash

basepath=$(cd "$(dirname "$0")/../" || exit; pwd)
cd "$basepath/" || exit

main() {
    mkdir -p build/
    cd build/ || error_exit "Failed to cd build/"

    cmake .. -DCMAKE_BUILD_TYPE=Release || {
        error_exit "Failed to cmake."
    }

    cmake --build . --config Release -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)" || {
        error_exit "Failed to build."
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
