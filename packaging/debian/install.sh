#!/bin/bash
set -e

echo "[+] Installing build dependencies..."
sudo apt-get update && sudo apt-get install -y cmake build-essential git

echo "[+] Cloning Keikaku (if not present)..."
if [ ! -d "keikaku-programming-language" ]; then
    git clone https://github.com/noobforanonymous/keikaku-programming-language.git
    cd keikaku-programming-language
else
    echo "Already inside repo or folder exists."
fi

echo "[+] Building Keikaku..."
mkdir -p build
cd build
cmake ..
make

echo "[+] Installing system-wide..."
sudo make install

echo "[+] Keikaku installed successfully! Try running 'keikaku'"
