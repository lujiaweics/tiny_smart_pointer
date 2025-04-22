#!/bin/bash

PROJECT_ROOT=$(dirname "$(realpath "$0")")/..

if [ -d "$PROJECT_ROOT/build" ]; then
    echo "Removing existing build directory..."
    rm -rf "$PROJECT_ROOT/build"
fi

echo "Creating new build directory..."
mkdir -p "$PROJECT_ROOT/build" || { echo "Failed to create build directory."; exit 1; }
cd "$PROJECT_ROOT/build" || { echo "Failed to enter build directory."; exit 1; }

echo "Configuring CMake project with coverage enabled..."
cmake .. -DENABLE_COVERAGE=True
if [ $? -ne 0 ]; then
    echo "CMake configuration failed."
    exit 1
fi

echo "Building the project..."
cmake --build .
if [ $? -ne 0 ]; then
    echo "Project build failed."
    exit 1
fi

echo "Running unit tests..."
./tiny_smart_pointer_unitest
if [ $? -ne 0 ]; then
    echo "Unit tests failed."
    exit 1
fi

echo "Unit tests completed successfully."