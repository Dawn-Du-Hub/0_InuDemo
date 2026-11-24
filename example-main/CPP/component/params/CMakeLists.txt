################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################
project(inuComponentParams LANGUAGES CXX VERSION 0.1.0)

################################################################################
# Include directories
################################################################################

include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/3drparty/yaml-cpp-0.7.0/include
    ${CMAKE_SOURCE_DIR}/3drparty/spdlog-1.x/include

)


set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/component/lib)

################################################################################
# Target
################################################################################

set(inuComponentParams_Library_SRCS
    ${CMAKE_SOURCE_DIR}/component/params/inuParams.cpp
)
add_library(${PROJECT_NAME} STATIC ${inuComponentParams_Library_SRCS})
