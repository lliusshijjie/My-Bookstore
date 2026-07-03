# ============================================================
#  TinyWebServer — 多阶段构建
#  Stage 1 (builder): 编译服务器二进制
#  Stage 2 (runtime): 最小运行时镜像
# ============================================================

# ---- Builder ----
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        g++ cmake make \
        libmysqlclient-dev \
        libgrpc++-dev \
        libprotobuf-dev \
        pkg-config \
        protobuf-compiler \
        protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN cmake -B build \
          -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_TESTS=OFF \
    && cmake --build build --parallel "$(nproc)"

# ---- Runtime ----
FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        libmysqlclient21 \
        libgrpc++1 \
        libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 拷贝编译产物和静态资源
COPY --from=builder /build/build/server /app/server
COPY --from=builder /build/build/inventory_grpc_server /app/inventory_grpc_server
COPY --from=builder /build/build/order_grpc_server /app/order_grpc_server
COPY --from=builder /build/build/user_grpc_server /app/user_grpc_server
COPY --from=builder /build/build/book_grpc_server /app/book_grpc_server
COPY --from=builder /build/client /app/client

# 服务器运行时通过 getcwd()+"/client" 确定文档根目录，
# 因此必须从 /app 目录启动二进制
WORKDIR /app

EXPOSE 9006 50051 50052 50053 50054

# 默认启动参数（可被 docker run / k8s args 覆盖）
# -p 端口  -l 日志模式(0同步/1异步)  -m 触发模式  -t 线程数  -s 连接池大小
ENTRYPOINT ["/app/server"]
CMD ["-p", "9006", "-l", "1", "-m", "3", "-t", "8", "-s", "8"]
