cmake_minimum_required(VERSION 3.0.2)
project(framework)

if(DEFINED PLATFROM_TYPE)
    if(${PLATFROM_TYPE} STREQUAL "ARM")
        inclide(arm64.cmake)
    endif()
else()
    include(x86.cmake)
endif()

