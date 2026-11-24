################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################
project(inu3drpartyFilter LANGUAGES CXX VERSION 0.1.0)

#include(../../CMake/Build.cmake)

################################################################################
# Compilation options 
################################################################################
find_package(OpenCV REQUIRED COMPONENTS)

################################################################################
# Include directories
################################################################################

include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/3drparty/filter/include
    ${OpenCV_INCLUDE_DIRS}
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/3drparty/filter/lib)

################################################################################
# Target
################################################################################

set(3drpartyFilter_Library_SRCS
    ${CMAKE_SOURCE_DIR}/3drparty/filter/src/depth2pc.cpp
    ${CMAKE_SOURCE_DIR}/3drparty/filter/src/depth2rgb.cpp
    ${CMAKE_SOURCE_DIR}/3drparty/filter/src/space_filter.cpp
    ${CMAKE_SOURCE_DIR}/3drparty/filter/src/temporal_filter.cpp

)
add_library(${PROJECT_NAME} STATIC ${3drpartyFilter_Library_SRCS})
