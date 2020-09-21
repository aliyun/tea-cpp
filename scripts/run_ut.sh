#!/bin/bash

basepath=$(cd `dirname $0`/../; pwd)

cd $basepath/

main() {
    mkdir -p cmake_build/ || {
       error_exit "Failed to mkdir cmake_build/"
    }

    cd cmake_build/ || {
       error_exit "Failed to cd cmake_build/"
    }

    cmake .. -DENABLE_UNIT_TESTS=ON || {
        error_exit "Failed to cmake."
    }

    cmake --build . || {
        error_exit "Failed to make."
    }

    ./tests || {
        error_exit "Failed to test."
    }
}

error_exit() {
    echo
    # shellcheck disable=SC2145
    echo "Error: $@"
    echo "----------------------------------------------------------------------"
    echo
    exit 1
}

main "$@"
exit 0
