cmake_minimum_required(VERSION 3.0.0)
project(share_memory VERSION 0.1.0)

include(CTest)

file(GLOB_RECURSE CPP_FILES "${CMAKE_CURRENT_LIST_DIR}/*cpp")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
