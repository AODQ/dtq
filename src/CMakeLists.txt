file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${dtq_SOURCE_DIR}/include/dtq/*.hpp")

add_executable(dtq source.cpp)
target_compile_features(dtq PRIVATE cxx_std_17)

#target_link_libraries(dtq PRIVATE
