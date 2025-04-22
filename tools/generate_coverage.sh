#!/bin/bash
set -e

BUILD_DIR=build
CMAKE_OPTIONS="-DENABLE_COVERAGE=ON"

cd ..
if [ -d "$BUILD_DIR" ]; then
  echo "Removing existing build directory..."
  rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR" || { echo "Failed to create build directory."; exit 1; }
cd "$BUILD_DIR" || { echo "Failed to enter build directory."; exit 1; }
echo "Configuring CMake project with coverage enabled..."
cmake .. $CMAKE_OPTIONS

echo "Building the project..."
cmake --build . -j4

echo "Running tests..."
ctest --output-on-failure

if grep -q "ENABLE_COVERAGE:BOOL=ON" CMakeCache.txt; then
  echo "Generating coverage report..."
  cmake --build . --target coverage
  echo "Coverage report generated at: $(pwd)/coverage_report/index.html"
else
  echo "Code coverage is not enabled."
fi