cmake_minimum_required(VERSION 3.20)
project(KVStorageTests)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(kvstorage_test
  test.cpp
)

target_link_libraries(kvstorage_test
  gtest
  gtest_main
  gmock
  gmock_main
)

include(GoogleTest)
gtest_discover_tests(kvstorage_test)