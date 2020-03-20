cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("[debug enabled]")
else()
  add_compile_definitions(NDEBUG)
endif()

project (
  dtq
  VERSION 0.0
  LANGUAGES CXX
  DESCRIPTION "Demo Toad Quill"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -fno-exceptions")

add_compile_definitions(
  VULKAN_HPP_NO_EXCEPTIONS
  SPDLOG_NO_EXCEPTIONS
)

## requires out of source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "build must be out of source")
endif()

## setup proejct
set(NAME dtq)
project(${NAME} CXX)

## Add dependents
find_package(Vulkan  REQUIRED FATAL_ERROR)
find_package(glfw3   REQUIRED FATAL_ERROR)
find_package(glm     REQUIRED FATAL_ERROR)
#find_package(glslang REQUIRED FATAL_ERROR)

## source list for application (src/include)
set(SOURCE_LIST
  "src/glfw.cpp"
  "src/graphicscontext.cpp"
  "src/source.cpp"
  "src/swapchain.cpp"
)
set(HEADER_LIST
  "src/glfw.hpp"
  "src/graphicscontext.hpp"
  "src/swapchain.hpp"
  "src/vulkan.hpp"
)

## add exceutable w/ source list and set compile options
add_executable(dtq ${SOURCE_LIST})
target_compile_features(dtq PRIVATE cxx_std_20)

## link dependents
target_link_libraries(dtq glfw glm vulkan glslang spdlog)

## add include/source directories , sources support necessary for (lamer) IDE
## users
target_include_directories(
  dtq
  PUBLIC ${GLFW_INCLUDE_DIRS}
  PUBLIC ${VULKAN_INCLUDE_DIRS}
  PUBLIC ${GLM_INCLUDE_DIRS}
)
target_sources(dtq PRIVATE ${SOURCE_LIST} ${HEADER_LIST})

## install binary files
install(
  TARGETS dtq
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT core
)

## TODO build shader files

## TODO install shader files
