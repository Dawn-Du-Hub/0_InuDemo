################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################
project(inuComponentIMU LANGUAGES CXX VERSION 0.1.0)

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

set(inuComponentIMU_Library_SRCS
    ${CMAKE_SOURCE_DIR}/component/imu/inuIMU.cpp
)
add_library(${PROJECT_NAME} STATIC ${inuComponentIMU_Library_SRCS})
