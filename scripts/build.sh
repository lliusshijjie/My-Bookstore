#!/bin/bash
# ============================================================
#  TinyWebServer 构建脚本
# ============================================================
# 用法:
#   ./build.sh              → 使用 Makefile 构建（Debug）
#   ./build.sh cmake        → 使用 CMake + Ninja/Make 构建（Debug）
#   DEBUG=0 ./build.sh      → 使用 Makefile 构建（Release，-O2）
#
# CMake 方式还支持:
#   mkdir build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#   make -j$(nproc)
#
# 可选环境变量:
#   BUILD_TYPE=Release ./build.sh cmake
#   BUILD_TESTS=1 ./build.sh cmake
#   ENABLE_REDIS=1 ./build.sh cmake
# ============================================================

# 自动切换到项目根目录（无论从哪个目录执行此脚本）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR" || exit 1

if [ "$1" = "cmake" ]; then
    BUILD_DIR="build"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || exit 1
    CMAKE_ARGS=(-DCMAKE_BUILD_TYPE="${BUILD_TYPE:-Debug}")
    if [ "${BUILD_TESTS:-0}" = "1" ]; then
        CMAKE_ARGS+=(-DBUILD_TESTS=ON)
    fi
    if [ "${ENABLE_REDIS:-0}" = "1" ]; then
        CMAKE_ARGS+=(-DTINYWEBSERVER_ENABLE_REDIS=ON)
    fi
    cmake .. "${CMAKE_ARGS[@]}"
    cmake --build . -j"$(nproc 2>/dev/null || echo 4)"
else
    make server
fi
