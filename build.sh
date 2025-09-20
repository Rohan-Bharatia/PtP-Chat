#!/bin/bash

set -e

echo "[+] Building..."
rm -rf build
mkdir build
cmake -G "Ninja" -S . -B build && echo "[+] CMake done" || (echo "[-] CMake failed" && exit 1)
cmake --build build --config Release && echo "[+] Build done" || (echo "[-] Build failed" && exit 1)

./build/bin/PtP_Chat && echo "[+] Running chat" || (echo "[-] Running chat failed" && exit 1)

echo "[+] Done!"
