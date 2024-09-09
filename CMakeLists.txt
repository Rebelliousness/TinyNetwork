#指定CMake最低版本号
cmake_minimum_required(VERSION 3.0)

#项目名称
project(tiny_network C CXX)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/base
    ${PROJECT_SOURCE_DIR}/include/net    
    ${PROJECT_SOURCE_DIR}/include/logger
    ${PROJECT_SOURCE_DIR}/include/timer
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/base SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net SRC_NET)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/logger SRC_LOGGER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/timer SRC_TIMER)


set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
)

#生成动态库
add_library(tiny_network SHARED ${SRC_BASE} ${SRC_NET})

target_link_libraries(tiny_network pthread)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include/tiny_network FILES_MATCHING PATTERN "*.h")

