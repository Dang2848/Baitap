#!/bin/bash
set -e

command -v arm-none-eabi-gcc >/dev/null || { echo "Thieu arm-none-eabi-gcc"; exit 1; }
command -v arm-none-eabi-objcopy >/dev/null || { echo "Thieu arm-none-eabi-objcopy"; exit 1; }
command -v arm-none-eabi-size >/dev/null || { echo "Thieu arm-none-eabi-size"; exit 1; }

if [ ! -f FreeRTOS/Kernel/tasks.c ]; then
    echo "Chua co FreeRTOS Kernel. Chay: ./setup_freertos.sh"
    exit 1
fi

./setup_freertos.sh
make check-kernel

echo "Preflight OK"
