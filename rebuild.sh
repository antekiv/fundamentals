#!/bin/bash
set -euo pipefail

# Define directories
BUILD_DIR="build"
SOURCE_DIR="$(pwd)"

# Remove the existing build directory
if [ -d "$BUILD_DIR" ]; then
  echo "Removing existing build directory..."
  rm -rf "$BUILD_DIR"
fi

# Create a new build directory
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure the project with CMake
cmake "$SOURCE_DIR"

# Build the project
cmake --build .

