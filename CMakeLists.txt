cmake_minimum_required(VERSION 3.20)
project(market_data_disseminator CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# CONFIG mode (not module) so protobuf::libprotobuf carries its Abseil deps
# transitively -- protobuf 35 is built on Abseil and the generated *.pb.cc call
# its logging/CHECK internals, which module-mode FindProtobuf does NOT link.
find_package(protobuf CONFIG REQUIRED)

# Shared library: transport + framing (+ generated protobuf, attached below).
add_library(md_common
  src/common/tcp_socket.cpp
  src/common/framing.cpp
  src/common/orderbook.cpp
  src/common/feed_reader.cpp
)

# Generate C++ sources from the .proto schemas and attach them to md_common.
# Headers land in CMAKE_CURRENT_BINARY_DIR as <name>.pb.h (import root = the
# proto dir), matching the bare #include "market_data.pb.h".
protobuf_generate(
  TARGET md_common
  LANGUAGE cpp
  IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/proto_buff_schemas
  PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
  PROTOS
    proto_buff_schemas/market_data.proto
    proto_buff_schemas/test_message.proto
)
target_include_directories(md_common PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/common
  ${CMAKE_CURRENT_BINARY_DIR}          # generated *.pb.h lands here
)
target_link_libraries(md_common PUBLIC protobuf::libprotobuf)

add_executable(md_server
  src/server/main.cpp
  src/server/event_handler.cpp
  src/server/translate.cpp
)
target_link_libraries(md_server PRIVATE md_common)

add_executable(md_client src/client/main.cpp)
target_link_libraries(md_client PRIVATE md_common)
