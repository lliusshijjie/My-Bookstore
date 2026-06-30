cmake_minimum_required(VERSION 3.10)
project(TinyWebServer VERSION 1.0 LANGUAGES CXX)

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

# ============================================================
#  外部依赖
# ============================================================
# pthread  — Linux 必须链接
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

# ============================================================
#  输出提示
# ============================================================
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
