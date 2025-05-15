cmake_minimum_required(VERSION 3.13)
project(PrintTypeInfo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM in: ${LLVM_DIR}")
message(STATUS "Found Clang in: ${Clang_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(PrintTypeInfo SHARED PrintTypeInfo.cpp)

target_link_libraries(PrintTypeInfo
  PRIVATE
  clangTooling
  clangBasic
  clangAST
  clangFrontend
  clangSerialization
)

set_target_properties(PrintTypeInfo PROPERTIES
  COMPILE_FLAGS "-fno-rtti"
)