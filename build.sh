#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "==> Configuring fLua (Release)..."
cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" -DCMAKE_BUILD_TYPE=Release

echo "==> Building fLua..."
cmake --build "${BUILD_DIR}" --config Release -j"$(nproc 2>/dev/null || echo 2)"

echo "==> Running tests..."
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "==> Build successful"
echo "    library: ${BUILD_DIR}/bin/libdifflua.so"
echo "    test:    ${BUILD_DIR}/bin/flua_test"
