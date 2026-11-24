################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################
project(inuComponentDataPath LANGUAGES CXX VERSION 0.1.0)

################################################################################
# Include directories
################################################################################

include_directories( ${PROJECT_NAME} PRIVATE
    ${INU_STREAMS_ROOT}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/3drparty/spdlog-1.x/include

)


set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/component/lib)

################################################################################
# Target
################################################################################

set(inuComponentDataPath_Library_SRCS
    ${CMAKE_SOURCE_DIR}/component/datapath/inuDataPath.cpp
)
add_library(${PROJECT_NAME} STATIC ${inuComponentDataPath_Library_SRCS})
