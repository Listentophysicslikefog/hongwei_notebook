cmake_minimum_required(VERSION 3.5.1)

project(main_board VERSION 8.0.0) # Apollo Version

set(DEPENCENCY_LIB_DIRS
${CMAKE_CURRENT_SOURCE_DIR}/lib/
)
add_definitions(-DGLOG_USE_GLOG_EXPORT)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../proto/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/fastrtps/
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/logger/
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/
  ${CMAKE_CURRENT_SOURCE_DIR}/../nlohmann_json/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../class_loader/
  ${CMAKE_CURRENT_SOURCE_DIR}/../transport/
  ${CMAKE_CURRENT_SOURCE_DIR}/../component/
  ${CMAKE_CURRENT_SOURCE_DIR}/../transport/rtps/
  ${CMAKE_CURRENT_SOURCE_DIR}/../transport/qos/
  ${CMAKE_CURRENT_SOURCE_DIR}/../transport/message/
  ${CMAKE_CURRENT_SOURCE_DIR}/../transport/common/
  ${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/communication/ 
  ${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/
  ${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/container/
  ${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/role/
  ${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/specific_manager/
  ${CMAKE_CURRENT_SOURCE_DIR}/../message/


)

link_directories(
${CMAKE_CURRENT_SOURCE_DIR}/../lib/
${DEPENCENCY_LIB_DIRS}/../)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/logger/  SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../proto/  SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}../ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../common/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../class_loader/ SRC_LIST_MAIN)


aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../node/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../scheduler/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../scheduler/policy/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../scheduler/common/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../task/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../time/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../timer/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../component/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../class_loader/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../class_loader/utility/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../class_loader/shared_library/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/specific_manager/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/communication/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/container/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/specific_manager/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../service_discovery/role/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../message/ SRC_LIST_MAIN)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../transport/qos/ SRC_LIST_MAIN)












aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../croutine/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../croutine/detail/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../transport/rtps/ SRC_LIST_MAIN)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../transport/message/ SRC_LIST_MAIN)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../transport/common/ SRC_LIST_MAIN)










#set(CMAKE_ASM_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/../croutine/detail/")

 
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g")
enable_language(CXX C ASM ASM-ATT)


message(STATUS "fewqfeqwfqg ${SRC_LIST_MAIN}")





message(STATUS "qwdqweq ${CMAKE_CURRENT_SOURCE_DIR}/../include/")

add_executable(main_board 
${CMAKE_CURRENT_SOURCE_DIR}/../croutine/detail/swap_x86_64.S
    ${SRC_LIST_MAIN} 
    ${CMAKE_CURRENT_SOURCE_DIR}/../state.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../binary.cc
   

)

target_link_libraries(main_board
    glog
    pthread
    protobuf
    gflags
    dl
    fastrtps
    fastcdr
    atomic
    uuid
    
)



#protoc -I=./ --proto_path=. --cpp_out=./ *.proto






