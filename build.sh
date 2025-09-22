#!/bin/bash

set -e

echo "[+] Building..."
rm -rf build
mkdir build
cmake -G "Ninja" -S . -B build && echo "[+] CMake done" || (echo "[-] CMake failed" && exit 1)
cmake --build build --config Release && echo "[+] Build done" || (echo "[-] Build failed" && exit 1)

cp ./build/bin/PtP_Chat ./PtP_Chat

echo "[+] Done!"
