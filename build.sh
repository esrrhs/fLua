#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "==> Configuring fLua (Release)..."
cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" -DCMAKE_BUILD_TYPE=Release

echo "==> Building fLua..."
cmake --build "${BUILD_DIR}" --config Release -j"$(nproc 2>/dev/null || echo 2)"

echo "==> Registered tests:"
TEST_LIST="$(ctest --test-dir "${BUILD_DIR}" -N)"
echo "${TEST_LIST}"
echo "${TEST_LIST}" | grep -q "pure_lua_test" || { echo "error: pure_lua_test was not registered"; exit 1; }
echo "${TEST_LIST}" | grep -q "cpp_test" || { echo "error: cpp_test was not registered"; exit 1; }

echo "==> Running tests..."
# Fail if no tests run. Verbose output is kept on success so CI logs show
# pure Lua, C++, bridge/cross, and error-path cases.
ctest --test-dir "${BUILD_DIR}" --output-on-failure --verbose --no-tests=error

echo "==> Build successful"
echo "    library: ${BUILD_DIR}/bin/libdifflua.so"
echo "    test:    ${BUILD_DIR}/bin/flua_test"
