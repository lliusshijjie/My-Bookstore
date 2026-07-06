#!/bin/bash
# 编译并启动所有服务：Redis + 4 个 gRPC 微服务 + HTTP 网关
# 用法: ./start.sh [--no-build]
# 可选: ENABLE_HNSW=1 ./start.sh
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"
LOG_DIR="/tmp/tws"
mkdir -p "$LOG_DIR"

SKIP_BUILD=0
for arg in "$@"; do
    case "$arg" in
        --no-build) SKIP_BUILD=1 ;;
    esac
done

# ---------- 编译 ----------
if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "[1/4] 编译全部目标 ..."
    cd "$ROOT" || exit 1
    CMAKE_ARGS=(-DTINYWEBSERVER_ENABLE_REDIS=ON -DCMAKE_BUILD_TYPE=Release)
    echo "  配置 CMake (启用 Redis 缓存) ..."
    if [ "${ENABLE_HNSW:-0}" = "1" ]; then
        echo "  启用 hnswlib 推荐后端 ..."
        CMAKE_ARGS+=(-DTINYWEBSERVER_ENABLE_HNSW=ON)
    fi
    cmake -B "$BUILD" "${CMAKE_ARGS[@]}" || exit 1
    cmake --build "$BUILD" -j"$(nproc)" || exit 1
    echo "  编译完成"
else
    echo "[1/4] 跳过编译 (--no-build)"
fi

# ---------- 基础设施 ----------
echo "[2/4] 检查 MySQL / Redis ..."
if ! mysqladmin ping -uroot -proot --silent 2>/dev/null; then
    echo "  MySQL 未运行，尝试启动 ..."
    sudo service mysql start 2>/dev/null || echo "  警告：请手动启动 MySQL"
fi
mysqladmin ping -uroot -proot --silent 2>/dev/null && echo "  MySQL: OK" || echo "  MySQL: 不可用"

if ! redis-cli ping >/dev/null 2>&1; then
    echo "  Redis 未运行，以后台方式启动 ..."
    redis-server --daemonize yes
fi
redis-cli ping >/dev/null 2>&1 && echo "  Redis: OK" || echo "  Redis: 不可用"

# ---------- 清理旧进程 ----------
echo "[3/4] 清理旧进程 ..."
# 注意：Linux 进程名会被截断为 15 字符，pkill -x 匹配不到长名，改用 -f 匹配完整命令行
pkill -f 'build/server'                2>/dev/null
pkill -f 'build/book_grpc_server'      2>/dev/null
pkill -f 'build/inventory_grpc_server' 2>/dev/null
pkill -f 'build/order_grpc_server'     2>/dev/null
pkill -f 'build/user_grpc_server'      2>/dev/null
sleep 1

# ---------- gRPC 微服务 ----------
echo "[4/4] 启动 gRPC 微服务与网关 ..."
cd "$ROOT" || exit 1

start_grpc() {
    local name="$1" bin="$2" port="$3"
    nohup "$BUILD/$bin" "0.0.0.0:$port" > "$LOG_DIR/$name.log" 2>&1 &
    echo "  $name -> 0.0.0.0:$port (pid $!)"
}

start_grpc "inventory_grpc" "inventory_grpc_server" 50051
start_grpc "order_grpc"     "order_grpc_server"     50052
start_grpc "user_grpc"      "user_grpc_server"      50053
start_grpc "book_grpc"      "book_grpc_server"      50054

# ---------- HTTP 网关 ----------
export REDIS_URL="tcp://127.0.0.1:6379"
export INVENTORY_GRPC_TARGET="127.0.0.1:50051"
export ORDER_GRPC_TARGET="127.0.0.1:50052"
export USER_GRPC_TARGET="127.0.0.1:50053"
export BOOK_GRPC_TARGET="127.0.0.1:50054"

nohup "$BUILD/server" > "$LOG_DIR/gateway.log" 2>&1 &
GATEWAY_PID=$!
echo "  gateway -> 0.0.0.0:9006 (pid $GATEWAY_PID)"

sleep 2
echo ""
echo "=== 监听端口 ==="
ss -ltn 2>/dev/null | grep -E ':(9006|50051|50052|50053|50054)' | sort

echo ""
echo "=== 健康检查 ==="
curl -s -o /dev/null -w "  gateway  /api/live  : HTTP %{http_code}\n" http://127.0.0.1:9006/api/live
curl -s -o /dev/null -w "  gateway  /api/ready : HTTP %{http_code}\n" http://127.0.0.1:9006/api/ready
curl -s -o /dev/null -w "  gateway  /api/books : HTTP %{http_code}\n" http://127.0.0.1:9006/api/books

echo ""
echo "日志目录: $LOG_DIR"
echo "停止全部: pkill -f 'build/server'; pkill -f 'build/book_grpc_server'; pkill -f 'build/inventory_grpc_server'; pkill -f 'build/order_grpc_server'; pkill -f 'build/user_grpc_server'"
