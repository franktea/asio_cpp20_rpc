cmake_minimum_required(VERSION 3.20)

project(asio_rpc_test)

add_compile_options(-std=c++2b -fcoroutines-ts)

include(FetchContent)
FetchContent_Declare(asio
  GIT_REPOSITORY    https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-22-0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_Populate(asio)
include_directories(${asio_SOURCE_DIR}/asio/include/)

FetchContent_Declare(cppack
  GIT_REPOSITORY    https://github.com/mikeloomisgg/cppack.git
  GIT_TAG master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_Populate(cppack)
include_directories(${cppack_SOURCE_DIR}/msgpack/include/)

add_executable(asio_rpc_client test_client.cpp)
add_executable(asio_rpc_server test_server.cpp)
