################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)


project(InuChipExampleUsbPort LANGUAGES CXX VERSION 0.1.0)
#include(../CMake/Build.cmake)

# Target
add_executable(${PROJECT_NAME}  demoInuUsbPort.cpp)

################################################################################
# Compilation options 
################################################################################



################################################################################
# Include directories
################################################################################
target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

################################################################################
# Link options 
################################################################################


################################################################################
# Link
################################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(InuSw_USB_Lib ${CMAKE_CURRENT_SOURCE_DIR}/libusb-1.0.lib)
else()
    find_library(InuSw_USB_Lib REQUIRED NO_DEFAULT_PATH NAMES usb-1.0 PATHS ${INU_STREAMS_ROOT}/bin)
endif()



if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs -pthread")
endif()


target_link_libraries(${PROJECT_NAME}
        ${InuSw_USB_Lib}
        )

