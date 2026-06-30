cmake_minimum_required(VERSION 3.10)

# 每个测试独立编译，只链接它依赖的源文件
# 不依赖 GTest，使用 <cassert> + 简单 main() 即可在 CI 中运行

set(TEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR})

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
