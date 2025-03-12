#!/bin/bash
set -e

BUILD_DIR=build
CMAKE_OPTIONS="-DENABLE_COVERAGE=ON"

cd ..
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir "$BUILD_DIR" && cd "$BUILD_DIR"
cmake .. ${CMAKE_OPTIONS}
cmake --build . -j4

ctest --output-on-failure

if grep -q "ENABLE_COVERAGE:BOOL=ON" CMakeCache.txt; then
    cmake --build . --target coverage
    echo "Coverage report generated at: $(pwd)/coverage_report/index.html"
fi