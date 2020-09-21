#!/bin/bash

basepath=$(cd `dirname $0`/../; pwd)

cd $basepath/

main() {
    mkdir -p cmake_build/
    cd cmake_build/
    cmake .. || {
        error_exit "Failed to cmake."
    }

    cmake --build . || {
        error_exit "Failed to make."
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
