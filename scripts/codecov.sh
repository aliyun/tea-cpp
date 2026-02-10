#!/bin/bash

basepath=$(cd "$(dirname "$0")/../" || exit; pwd)
cd "$basepath/" || exit

mkdir -p build/
cd build/ || exit
cmake -DENABLE_UNIT_TESTS=ON -DENABLE_COVERAGE=ON .. || exit
cmake --build . -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)" || exit
ctest --output-on-failure || exit

echo '--------- generate initial info ---------------- '
rm -rf ./coverage
mkdir -p coverage
lcov -z
lcov --directory . --capture --output-file coverage.info

echo '--------- run test ---------------- '
ctest --verbose --coverage

echo '--------- generate post info ---------------- '
lcov --remove coverage.info '/usr/*' "${HOME}/.cache/*" --output-file coverage.info
lcov --list coverage.info

echo '--------- generate html report ---------------- '
genhtml -o coverage --prefix="$PWD/coverage/" coverage.info

echo "check report: $PWD/coverage/index.html"
