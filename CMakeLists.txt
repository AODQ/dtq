cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)

project (
  dtq
  VERSION 0.0
  DESCRIPTION "Demo Toad Quill"
)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS ON)
  set(CXX_STANDARD 20)
endif()

## requires out of source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "build must be out of source")
endif()

set(NAME dtq)

project(${NAME} CXX)


add_subdirectory(src)
