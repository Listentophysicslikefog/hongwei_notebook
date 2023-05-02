cmake_minimum_required(VERSION 3.0.0)
project(framework)

add_compile_options(-std=c++17 -g)
add_definitions(-fPIC)
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/shm_inc
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/shm_src
    ${CMAKE_CURRENT_LIST_DIR}/yaml-cpp/
)



file(GLOB_RECURSE SHM_CPP_FILES 
"${CMAKE_CURRENT_LIST_DIR}/shm_src/*cpp"
"${CMAKE_CURRENT_LIST_DIR}/lockless.cpp"
)
set_property(GLOBAL APPEND PROPERTY SOURCE_LIST ${SHM_CPP_FILES})
get_property(SRC_LIST GLOBAL PROPERTY SOURCE_LIST)
add_library(share_memory SHARED ${SRC_LIST})

target_link_libraries(share_memory
${CMAKE_CURRENT_LIST_DIR}/lib/libyaml-cpp.so
)
