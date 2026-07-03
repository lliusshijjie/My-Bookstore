CXX ?= g++
PROTOC ?= protoc
GRPC_CPP_PLUGIN ?= grpc_cpp_plugin
GRPC_OUT_DIR ?= build/generated

CXXFLAGS += -std=c++17 -Wall -Wextra -I.

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g -O0
else
    CXXFLAGS += -O2
endif

SRCS = src/core/main.cpp src/timer/lst_timer.cpp src/net/http/http_conn.cpp \
       src/app/recommend/book_vector_index.cpp \
       src/app/repository/mysql/mysql_repositories.cpp \
       src/db/user_cache.cpp src/log/log.cpp src/db/sql_connection_pool.cpp \
       src/core/webserver.cpp src/core/config.cpp

PROTO_FILES = proto/common/v1/common.proto \
              proto/user/v1/user.proto \
              proto/book/v1/book.proto \
              proto/inventory/v1/inventory.proto \
              proto/order/v1/order.proto

.PHONY: server proto-check grpc-stubs clean

server: $(SRCS)
	$(CXX) -o server $^ $(CXXFLAGS) -lpthread -lmysqlclient

proto-check:
	@if command -v protoc >/dev/null 2>&1; then \
		protoc --proto_path=. --descriptor_set_out=/tmp/bookstore.protoset $(PROTO_FILES); \
	else \
		echo "protoc not found; install protobuf-compiler to validate proto syntax"; \
	fi

grpc-stubs:
	@if ! command -v $(PROTOC) >/dev/null 2>&1; then \
		echo "protoc not found; install protobuf-compiler"; \
		exit 1; \
	fi
	@if ! command -v $(GRPC_CPP_PLUGIN) >/dev/null 2>&1; then \
		echo "grpc_cpp_plugin not found; install protobuf-compiler-grpc"; \
		exit 1; \
	fi
	mkdir -p $(GRPC_OUT_DIR)
	$(PROTOC) --proto_path=. --cpp_out=$(GRPC_OUT_DIR) proto/common/v1/common.proto
	$(PROTOC) --proto_path=. --cpp_out=$(GRPC_OUT_DIR) --grpc_out=$(GRPC_OUT_DIR) \
		--plugin=protoc-gen-grpc=$$(command -v $(GRPC_CPP_PLUGIN)) \
		proto/user/v1/user.proto
	$(PROTOC) --proto_path=. --cpp_out=$(GRPC_OUT_DIR) --grpc_out=$(GRPC_OUT_DIR) \
		--plugin=protoc-gen-grpc=$$(command -v $(GRPC_CPP_PLUGIN)) \
		proto/book/v1/book.proto
	$(PROTOC) --proto_path=. --cpp_out=$(GRPC_OUT_DIR) --grpc_out=$(GRPC_OUT_DIR) \
		--plugin=protoc-gen-grpc=$$(command -v $(GRPC_CPP_PLUGIN)) \
		proto/inventory/v1/inventory.proto
	$(PROTOC) --proto_path=. --cpp_out=$(GRPC_OUT_DIR) --grpc_out=$(GRPC_OUT_DIR) \
		--plugin=protoc-gen-grpc=$$(command -v $(GRPC_CPP_PLUGIN)) \
		proto/order/v1/order.proto

clean:
	rm -f server
