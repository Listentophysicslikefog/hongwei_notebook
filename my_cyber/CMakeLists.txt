cmake_minimum_required(VERSION 3.5.1)

project(glog_test VERSION 8.0.0) # Apollo Version

set(DEPENCENCY_LIB_DIRS
${CMAKE_CURRENT_SOURCE_DIR}/lib/
)
add_definitions(-DGLOG_USE_GLOG_EXPORT)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/proto/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/logger/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
  ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json/include/
)

link_directories(
${CMAKE_CURRENT_SOURCE_DIR}/lib/
${DEPENCENCY_LIB_DIRS})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/logger/  SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/proto/  SRC_LIST)
#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/common/ SRC_LIST)


message(STATUS "qwdqweq ${TARGET_NAME}")

add_executable(glog_test  main.cpp binary.cc ${SRC_LIST}
)

target_link_libraries(glog_test
    glog
    pthread
    protobuf
)



#protoc -I=./ --proto_path=. --cpp_out=./ *.proto






