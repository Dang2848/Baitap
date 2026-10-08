#!/bin/bash
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
KERNEL_DIR="$ROOT_DIR/FreeRTOS/Kernel"
KERNEL_TAG="V11.3.0"
KERNEL_COMMIT="9b777ae5c5b8e9e456065a00294d1e5f5f9facf5"
REPO="https://github.com/FreeRTOS/FreeRTOS-Kernel.git"

mkdir -p "$ROOT_DIR/FreeRTOS"

if [ -f "$KERNEL_DIR/tasks.c" ] && [ -f "$KERNEL_DIR/portable/GCC/ARM_CM3/port.c" ]; then
    echo "FreeRTOS Kernel da co san."
    exit 0
fi

rm -rf "$KERNEL_DIR"

echo "Dang tai FreeRTOS Kernel $KERNEL_TAG..."
git clone --depth 1 --branch "$KERNEL_TAG" "$REPO" "$KERNEL_DIR"

cd "$KERNEL_DIR"
ACTUAL="$(git rev-parse HEAD)"
if [ "$ACTUAL" != "$KERNEL_COMMIT" ]; then
    echo "Loi: commit kernel khong dung."
    echo "Can:  $KERNEL_COMMIT"
    echo "Nhan: $ACTUAL"
    exit 1
fi

echo "FreeRTOS Kernel $KERNEL_TAG da san sang."
