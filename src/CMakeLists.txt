file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../include/*.hpp")

add_executable(dtq ${SOURCE_LIST})
target_compile_features(dtq PRIVATE cxx_std_20)
target_sources(dtq PRIVATE ${SOURCE_LIST} ${HEADER_LIST})

#target_link_libraries(dtq PRIVATE
