cmake_minimum_required(VERSION 3.10)

# 每个测试独立编译，只链接它依赖的源文件
# 不依赖 GTest，使用 <cassert> + 简单 main() 即可在 CI 中运行

set(TEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR})
find_package(Threads REQUIRED)

# --- test_url_params ---
add_executable(test_url_params test_url_params.cpp)
target_include_directories(test_url_params PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_url_params PRIVATE cxx_std_17)
add_test(NAME url_params COMMAND test_url_params)

# --- test_block_queue ---
add_executable(test_block_queue test_block_queue.cpp)
target_include_directories(test_block_queue PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_block_queue PRIVATE cxx_std_17)
target_link_libraries(test_block_queue PRIVATE Threads::Threads)
add_test(NAME block_queue COMMAND test_block_queue)

# --- test_semaphore ---
add_executable(test_semaphore test_semaphore.cpp)
target_include_directories(test_semaphore PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_semaphore PRIVATE cxx_std_17)
target_link_libraries(test_semaphore PRIVATE Threads::Threads)
add_test(NAME semaphore COMMAND test_semaphore)

# --- test_http_router ---
add_executable(test_http_router test_http_router.cpp)
target_include_directories(test_http_router PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_http_router PRIVATE cxx_std_17)
add_test(NAME http_router COMMAND test_http_router)

# --- test_static_root ---
add_executable(test_static_root test_static_root.cpp)
target_include_directories(test_static_root PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_static_root PRIVATE cxx_std_17)
add_test(NAME static_root COMMAND test_static_root)

# --- test_book_catalog ---
add_executable(test_book_catalog test_book_catalog.cpp)
target_include_directories(test_book_catalog PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_book_catalog PRIVATE cxx_std_17)
add_test(NAME book_catalog COMMAND test_book_catalog)

# --- test_ecommerce_flow ---
add_executable(test_ecommerce_flow test_ecommerce_flow.cpp)
target_include_directories(test_ecommerce_flow PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_ecommerce_flow PRIVATE cxx_std_17)
add_test(NAME ecommerce_flow COMMAND test_ecommerce_flow)

# --- test_database_schema ---
add_executable(test_database_schema test_database_schema.cpp)
target_include_directories(test_database_schema PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_database_schema PRIVATE cxx_std_17)
add_test(NAME database_schema COMMAND test_database_schema)

# --- test_repository_services ---
add_executable(test_repository_services test_repository_services.cpp)
target_include_directories(test_repository_services PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_repository_services PRIVATE cxx_std_17)
add_test(NAME repository_services COMMAND test_repository_services)

# --- test_order_inventory_client ---
add_executable(test_order_inventory_client test_order_inventory_client.cpp)
target_include_directories(test_order_inventory_client PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_order_inventory_client PRIVATE cxx_std_17)
add_test(NAME order_inventory_client COMMAND test_order_inventory_client)

# --- test_order_gateway_client ---
add_executable(test_order_gateway_client test_order_gateway_client.cpp)
target_include_directories(test_order_gateway_client PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_order_gateway_client PRIVATE cxx_std_17)
add_test(NAME order_gateway_client COMMAND test_order_gateway_client)

# --- test_mysql_repository_wiring ---
add_executable(test_mysql_repository_wiring test_mysql_repository_wiring.cpp)
target_include_directories(test_mysql_repository_wiring PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_mysql_repository_wiring PRIVATE cxx_std_17)
target_link_libraries(test_mysql_repository_wiring PRIVATE mysqlclient)
add_test(NAME mysql_repository_wiring COMMAND test_mysql_repository_wiring)

# --- test_proto_contracts ---
add_executable(test_proto_contracts test_proto_contracts.cpp)
target_include_directories(test_proto_contracts PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_proto_contracts PRIVATE cxx_std_17)
add_test(NAME proto_contracts COMMAND test_proto_contracts)

# --- test_inventory_gateway_client ---
add_executable(test_inventory_gateway_client test_inventory_gateway_client.cpp)
target_include_directories(test_inventory_gateway_client PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_inventory_gateway_client PRIVATE cxx_std_17)
add_test(NAME inventory_gateway_client COMMAND test_inventory_gateway_client)

# --- test_inventory_grpc_wiring ---
add_executable(test_inventory_grpc_wiring test_inventory_grpc_wiring.cpp)
target_include_directories(test_inventory_grpc_wiring PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_inventory_grpc_wiring PRIVATE cxx_std_17)
add_test(NAME inventory_grpc_wiring COMMAND test_inventory_grpc_wiring)

# --- test_inventory_runtime_config ---
add_executable(test_inventory_runtime_config test_inventory_runtime_config.cpp)
target_include_directories(test_inventory_runtime_config PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_inventory_runtime_config PRIVATE cxx_std_17)
add_test(NAME inventory_runtime_config COMMAND test_inventory_runtime_config)

# --- test_order_grpc_wiring ---
add_executable(test_order_grpc_wiring test_order_grpc_wiring.cpp)
target_include_directories(test_order_grpc_wiring PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_order_grpc_wiring PRIVATE cxx_std_17)
add_test(NAME order_grpc_wiring COMMAND test_order_grpc_wiring)

# --- test_user_book_gateway_client ---
add_executable(test_user_book_gateway_client test_user_book_gateway_client.cpp)
target_include_directories(test_user_book_gateway_client PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_user_book_gateway_client PRIVATE cxx_std_17)
add_test(NAME user_book_gateway_client COMMAND test_user_book_gateway_client)

# --- test_user_book_grpc_wiring ---
add_executable(test_user_book_grpc_wiring test_user_book_grpc_wiring.cpp)
target_include_directories(test_user_book_grpc_wiring PRIVATE ${TEST_INCLUDE_DIR})
target_compile_features(test_user_book_grpc_wiring PRIVATE cxx_std_17)
add_test(NAME user_book_grpc_wiring COMMAND test_user_book_grpc_wiring)
