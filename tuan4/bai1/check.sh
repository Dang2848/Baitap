#!/bin/bash
set -e

./preflight.sh
make clean
make
make info

echo
if [ -f freertos_bai1.bin ]; then
    echo "BUILD OK: freertos_bai1.bin da duoc tao."
else
    echo "BUILD FAIL"
    exit 1
fi
