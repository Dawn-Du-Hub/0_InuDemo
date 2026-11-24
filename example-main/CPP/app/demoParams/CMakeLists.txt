################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################

cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)


project(InuChipExampleParams LANGUAGES CXX VERSION 0.1.0)


# Target
add_executable(${PROJECT_NAME}  demoInuParams.cpp)

################################################################################
# Compilation options 
################################################################################


################################################################################
# Include directories
################################################################################

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/3drparty/spdlog-1.x/include

)

################################################################################
# Link options 
################################################################################
#link_directories(${CMAKE_SOURCE_DIR}/3drparty/yaml-cpp-0.7.0/lib/x64)


#message(" link_directories = ${link_directories}")


#target_link_directories(${PROJECT_NAME} PRIVATE /private/dir INTERFACE /interface/dir)

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
        ${Yaml_Lib}
        ${InuSw_Streams_Lib}
        ${InuSw_CommonUtils_Lib}
        spdlog)
