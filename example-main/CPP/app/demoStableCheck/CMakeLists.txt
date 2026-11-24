################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################

cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)


project(InuChipExampleStabeCheck LANGUAGES CXX VERSION 0.1.0)
#include(../CMake/Build.cmake)

# Target
add_executable(${PROJECT_NAME}  demoInuStability.cpp)

################################################################################
# Compilation options 
################################################################################

find_package(OpenCV REQUIRED COMPONENTS)



################################################################################
# Include directories
################################################################################
#SET(INU_STREAMS_ROOT "C:/Program Files/Inuitive/InuDev")
target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${INU_STREAMS_ROOT}/include
    ${CMAKE_SOURCE_DIR}/3drparty/spdlog-1.x/include
    ${OpenCV_INCLUDE_DIRS}
)

################################################################################
# Link options 
################################################################################


################################################################################
# Link
################################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(Yaml_Lib ${CMAKE_SOURCE_DIR}/3drparty/yaml-cpp-0.7.0/lib/x64/yaml-cpp.lib)
    else()
        set(Yaml_Lib ${CMAKE_SOURCE_DIR}/3drparty/yaml-cpp-0.7.0/lib/x64/yaml-cppd.lib)
    endif()
else()
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(Yaml_Lib ${CMAKE_SOURCE_DIR}/3drparty/yaml-cpp-0.7.0/lib/linux/arm/libyaml-cpp.a)
    else()
        set(Yaml_Lib ${CMAKE_SOURCE_DIR}/3drparty/yaml-cpp-0.7.0/lib/linux/x64/libyaml-cpp.a)
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_library(InuSw_Streams_Lib REQUIRED NAMES InuStreamsd PATHS ${INU_LIBRARY_DIR})
    find_library(InuSw_CommonUtils_Lib REQUIRED NAMES InuCommonUtilitiesd PATHS ${INU_LIBRARY_DIR})
else()
    find_library(InuSw_Streams_Lib REQUIRED NAMES InuStreams PATHS ${INU_LIBRARY_DIR})
    find_library(InuSw_CommonUtils_Lib REQUIRED NAMES InuCommonUtilities PATHS ${INU_LIBRARY_DIR})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs -pthread")
endif()

target_link_libraries(${PROJECT_NAME}
        inuComponentParams
        ${OpenCV_LIBRARIES}
        ${Yaml_Lib}
        ${InuSw_Streams_Lib}
        ${InuSw_CommonUtils_Lib}
        inuComponentSensor inuComponentDataPath inuComponentDepth inuComponentRGB spdlog
    )