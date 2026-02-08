#!/bin/bash
set -e

# Ensure we are in the project root
cd "$(dirname "$0")/../../"

echo "[+] Building Keikaku for Windows (x64)..."

# Create a separate build directory for Windows
mkdir -p build-win
cd build-win

# Run CMake with the MinGW toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../packaging/windows/Toolchain-mingw.cmake ..

# Compile
make

echo "[+] Build complete!"
echo "[+] Executable is at: build-win/keikaku.exe"
ls -l keikaku.exe
