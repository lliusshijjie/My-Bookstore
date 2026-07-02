cmake_minimum_required(VERSION 3.10)
project(TinyWebServer VERSION 1.0 LANGUAGES CXX)

set(PROTO_FILES
    proto/common/v1/common.proto
    proto/user/v1/user.proto
    proto/book/v1/book.proto
    proto/inventory/v1/inventory.proto
    proto/order/v1/order.proto
)

# ============================================================
#  C++ 标准
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
#  编译选项（与原 Makefile 对齐）
# ============================================================
# DEBUG=1  → Debug   : -g -O0
# DEBUG=0  → Release : -O2
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Build type: Debug or Release" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wextra")

# ============================================================
#  源文件
# ============================================================
set(SOURCES
    src/core/main.cpp
    src/core/webserver.cpp
    src/core/config.cpp
    src/timer/lst_timer.cpp
    src/net/http/http_conn.cpp
    src/app/repository/mysql/mysql_repositories.cpp
    src/db/user_cache.cpp
    src/log/log.cpp
    src/db/sql_connection_pool.cpp
)

# ============================================================
#  可执行文件
# ============================================================
add_executable(server ${SOURCES})

# ============================================================
#  头文件搜索路径
# ============================================================
# PRIVATE  — 仅编译 server 时使用（src/ 内的模块间引用）
# PUBLIC   — 允许外部消费者通过 include/tinyweb/ 引用公共 API
target_include_directories(server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ============================================================
#  单元测试（可选，cmake .. -DBUILD_TESTS=ON 启用）
# ============================================================
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

find_program(PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
if(PROTOC_EXECUTABLE)
    add_custom_target(check_proto_contracts
        COMMAND ${PROTOC_EXECUTABLE}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
                --descriptor_set_out=${CMAKE_CURRENT_BINARY_DIR}/bookstore.protoset
                ${PROTO_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Validating Protobuf contracts")
else()
    add_custom_target(check_proto_contracts
        COMMAND ${CMAKE_COMMAND} -E echo
                "protoc not found - install protobuf-compiler to validate proto syntax"
        COMMENT "Skipping Protobuf syntax validation")
endif()

# ============================================================
#  外部依赖
# ============================================================
# pthread  — Linux 必须链接
find_package(Protobuf QUIET)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GRPCPP QUIET grpc++)
endif()

set(TINYWEBSERVER_GRPC_AVAILABLE OFF)
if(PROTOC_EXECUTABLE AND GRPC_CPP_PLUGIN_EXECUTABLE AND Protobuf_FOUND AND GRPCPP_FOUND)
    set(TINYWEBSERVER_GRPC_AVAILABLE ON)

    set(GENERATED_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
    set(COMMON_PROTO_SRC ${GENERATED_PROTO_DIR}/proto/common/v1/common.pb.cc)
    set(COMMON_PROTO_HDR ${GENERATED_PROTO_DIR}/proto/common/v1/common.pb.h)
    set(INVENTORY_PROTO_SRC ${GENERATED_PROTO_DIR}/proto/inventory/v1/inventory.pb.cc)
    set(INVENTORY_PROTO_HDR ${GENERATED_PROTO_DIR}/proto/inventory/v1/inventory.pb.h)
    set(INVENTORY_GRPC_SRC ${GENERATED_PROTO_DIR}/proto/inventory/v1/inventory.grpc.pb.cc)
    set(INVENTORY_GRPC_HDR ${GENERATED_PROTO_DIR}/proto/inventory/v1/inventory.grpc.pb.h)

    add_custom_command(
        OUTPUT ${COMMON_PROTO_SRC} ${COMMON_PROTO_HDR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_PROTO_DIR}
        COMMAND ${PROTOC_EXECUTABLE}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
                --cpp_out=${GENERATED_PROTO_DIR}
                proto/common/v1/common.proto
        DEPENDS proto/common/v1/common.proto
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating common protobuf stubs")

    add_custom_command(
        OUTPUT ${INVENTORY_PROTO_SRC} ${INVENTORY_PROTO_HDR}
               ${INVENTORY_GRPC_SRC} ${INVENTORY_GRPC_HDR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_PROTO_DIR}
        COMMAND ${PROTOC_EXECUTABLE}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
                --cpp_out=${GENERATED_PROTO_DIR}
                --grpc_out=${GENERATED_PROTO_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
                proto/inventory/v1/inventory.proto
        DEPENDS proto/inventory/v1/inventory.proto proto/common/v1/common.proto
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating inventory gRPC stubs")

    add_library(inventory_grpc_proto
        ${COMMON_PROTO_SRC}
        ${INVENTORY_PROTO_SRC}
        ${INVENTORY_GRPC_SRC}
    )
    target_include_directories(inventory_grpc_proto
        PUBLIC
            ${GENERATED_PROTO_DIR}
            ${Protobuf_INCLUDE_DIRS}
            ${GRPCPP_INCLUDE_DIRS}
    )
    target_link_libraries(inventory_grpc_proto
        PUBLIC
            ${Protobuf_LIBRARIES}
            ${GRPCPP_LDFLAGS}
    )

    add_library(inventory_grpc_service
        src/app/grpc/inventory_grpc_service.cpp
        src/app/client/grpc_inventory_client.cpp
    )
    target_include_directories(inventory_grpc_service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(inventory_grpc_service PUBLIC inventory_grpc_proto)

    add_executable(inventory_grpc_server src/app/grpc/inventory_server_main.cpp)
    target_include_directories(inventory_grpc_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(inventory_grpc_server PRIVATE inventory_grpc_service)

    if(BUILD_TESTS)
        add_executable(test_inventory_grpc_service test/test_inventory_grpc_service.cpp)
        target_include_directories(test_inventory_grpc_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_features(test_inventory_grpc_service PRIVATE cxx_std_17)
        target_link_libraries(test_inventory_grpc_service PRIVATE inventory_grpc_service)
        add_test(NAME inventory_grpc_service COMMAND test_inventory_grpc_service)
    endif()

    target_link_libraries(server PRIVATE inventory_grpc_service)
    target_compile_definitions(server PRIVATE TINYWEBSERVER_ENABLE_GRPC)
else()
    message(STATUS "Inventory gRPC targets disabled. Need protoc, grpc_cpp_plugin, Protobuf, and grpc++ pkg-config metadata.")
endif()

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)

# mysqlclient  — 数据库连接
# 如果 CMake 找不到，可通过 -DMYSQL_LIBRARY=... -DMYSQL_INCLUDE_DIR=... 指定
find_library(MYSQL_LIBRARY NAMES mysqlclient
    PATHS /usr/lib /usr/lib64 /usr/lib/x86_64-linux-gnu
          /usr/local/lib /usr/local/mysql/lib)
find_path(MYSQL_INCLUDE_DIR NAMES mysql/mysql.h
    PATHS /usr/include /usr/local/include /usr/local/mysql/include)

if(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIR)
    target_link_libraries(server PRIVATE ${MYSQL_LIBRARY})
    target_include_directories(server PRIVATE ${MYSQL_INCLUDE_DIR})
    message(STATUS "Found mysqlclient: ${MYSQL_LIBRARY}")
else()
    message(WARNING "mysqlclient not found — server will link but DB features may fail at runtime.\n"
                    "Install: apt install libmysqlclient-dev  (Debian/Ubuntu)\n"
                    "         yum install mysql-devel         (RHEL/CentOS)\n"
                    "Or pass: -DMYSQL_LIBRARY=/path/to/libmysqlclient.so -DMYSQL_INCLUDE_DIR=/path/to/mysql/include")
    # 仍然尝试链接 mysqlclient，让构建在其存在时成功
    target_link_libraries(server PRIVATE mysqlclient)
endif()

if(TARGET inventory_grpc_server)
    target_sources(inventory_grpc_server PRIVATE
        src/app/repository/mysql/mysql_repositories.cpp
        src/db/sql_connection_pool.cpp
        src/log/log.cpp
    )
    target_link_libraries(inventory_grpc_server PRIVATE Threads::Threads)
    if(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIR)
        target_link_libraries(inventory_grpc_server PRIVATE ${MYSQL_LIBRARY})
        target_include_directories(inventory_grpc_server PRIVATE ${MYSQL_INCLUDE_DIR})
    else()
        target_link_libraries(inventory_grpc_server PRIVATE mysqlclient)
    endif()
endif()

# ============================================================
#  输出提示
# ============================================================
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
