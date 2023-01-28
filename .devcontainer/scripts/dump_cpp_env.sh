#!/bin/sh

set -x

cat /etc/os-release
cmake --version
gcc -v
clang -v
ld -v
python3 --version
vcpkg version
