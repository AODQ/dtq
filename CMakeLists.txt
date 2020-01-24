cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)

project (
  dtq
  VERSION 0.0
  LANGUAGES CXX
  DESCRIPTION "Demo Toad Quill"
)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

## requires out of source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "build must be out of source")
endif()

## setup proejct
set(NAME dtq)
project(${NAME} CXX)

## Add dependents
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glfw3  REQUIRED FATAL_ERROR)

## build application (src/include)
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/*.hpp")

add_executable(dtq ${SOURCE_LIST})
target_compile_features(dtq PRIVATE cxx_std_20)

## link dependents
target_link_libraries(dtq glfw)

## add include/source directories , sources support necessary for (lamer) IDE
## users
target_include_directories(
  dtq
  PUBLIC ${GLFW_INCLUDE_DIRS}
  PUBLIC ${VULKAN_INCLUDE_DIRS}
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
