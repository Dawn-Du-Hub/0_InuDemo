################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################

cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)


project(InuChipExampleSensor LANGUAGES CXX VERSION 0.1.0)
#include(../CMake/Build.cmake)

# Target
add_executable(${PROJECT_NAME}  demoInuSensor.cpp)

################################################################################
# Compilation options 
################################################################################


################################################################################
# Include directories
################################################################################
#SET(INU_STREAMS_ROOT "C:/Program Files/Inuitive/InuDev")
target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${INU_STREAMS_ROOT}/include
    ${CMAKE_SOURCE_DIR}/3drparty/spdlog-1.x/include
)

################################################################################
# Link options 
################################################################################


################################################################################
# Link
################################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_library(InuSw_Streams_Lib REQUIRED NAMES InuStreamsd PATHS ${INU_LIBRARY_DIR})
    find_library(InuSw_CommonUtils_Lib REQUIRED NAMES InuCommonUtilitiesd PATHS ${INU_LIBRARY_DIR})
else()
    find_library(InuSw_Streams_Lib REQUIRED NAMES InuStreams PATHS ${INU_LIBRARY_DIR})
    find_library(InuSw_CommonUtils_Lib REQUIRED NAMES InuCommonUtilities PATHS ${INU_LIBRARY_DIR})
endif()

target_link_libraries(${PROJECT_NAME}
        ${InuSw_Streams_Lib}
        ${InuSw_CommonUtils_Lib}
        inuComponentSensor spdlog)
