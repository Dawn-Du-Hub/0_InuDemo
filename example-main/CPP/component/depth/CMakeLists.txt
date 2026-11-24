################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################
project(inuComponentDepth LANGUAGES CXX VERSION 0.1.0)

#include(../CMake/Build.cmake)

################################################################################
# Include directories
################################################################################

include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${INU_STREAMS_ROOT}/include
    ${CMAKE_SOURCE_DIR}/3drparty/spdlog-1.x/include

)


set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/component/lib)

################################################################################
# Target
################################################################################

set(inuComponentDepth_Library_SRCS
    ${CMAKE_SOURCE_DIR}/component/depth/inuDepth.cpp
    ${CMAKE_SOURCE_DIR}/component/depth/inuDepthFilter.cpp

)
add_library(${PROJECT_NAME} STATIC ${inuComponentDepth_Library_SRCS})
