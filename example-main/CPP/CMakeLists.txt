################################################################################
# Inuchip C++ Example cmake project 
# 
# Copyright (C) 2023 All rights reserved to InuChip
################################################################################

cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(InuChipExample LANGUAGES CXX VERSION 0.1.0)

include(CMake/Build.cmake)

message("if you find 'it set OpencCV_FOUND to FALSE .....'")
message("One reason is your VC version is to high, you can open OpenCV_RUNTIME setting in CMakeLists.txt!")

set(OpenCV_RUNTIME vc15)

add_subdirectory(component/params)
add_subdirectory(component/datapath)
add_subdirectory(component/sensor)
add_subdirectory(component/logger)
add_subdirectory(component/rgb)
add_subdirectory(component/depth)
add_subdirectory(component/ir)
add_subdirectory(component/pcl)
add_subdirectory(component/imu)
add_subdirectory(component/tempera)
add_subdirectory(component/fe)
add_subdirectory(component/feature)
add_subdirectory(component/slam)
add_subdirectory(component/cnn)






if(BUILD_PARAMS_DEMO)
    add_subdirectory(app/demoParams)
endif()

if(BUILD_SENSOR_DEMO)
    add_subdirectory(app/demoSensor)
endif()

if(BUILD_RGB_DEMO)
    add_subdirectory(app/demoRGB)
endif()

if(BUILD_DEPTH_DEMO)
    add_subdirectory(app/demoDepth)
endif()

if(BUILD_IR_DEMO)
    add_subdirectory(app/demoIR)
endif()

if(BUILD_RECONNECT_DEMO)
    add_subdirectory(app/demoReconnect)
endif()

if(BUILD_DISPARITY_DEMO)
    add_subdirectory(app/demoDisparity)
endif()

if(BUILD_2DEPTH_DEMO)
    add_subdirectory(app/demo2Depth)
endif()

if(BUILD_D2C_DEMO)
    add_subdirectory(app/demoD2C)
endif()

if(BUILD_PCL_DEMO)
    add_subdirectory(app/demoPCL)
endif()

if(BUILD_DEPTHPCL_DEMO)
    add_subdirectory(app/demoDepthPCL)
endif()

if(BUILD_3drpartyFilter)
    add_subdirectory(3drparty/filter)
endif()

if(BUILD_DEPTH3PARTYFILTER_DEMO)
    add_subdirectory(app/demoDepth3partyFilter)
endif()

if(BUILD_IMU_DEMO)
    add_subdirectory(app/demoIMU)
endif()

if(BUILD_TEMPERA_DEMO)
    add_subdirectory(app/demoTempera)
endif()

if(BUILD_stableCheck)
    add_subdirectory(app/demoStableCheck)
endif()

if(BUILD_FISHEYE_DEMO)
    add_subdirectory(app/demoFisheye)
endif()

if(BUILD_FEATURE_DEMO)
    add_subdirectory(app/demoFeature6DOF)
endif()

if(BUILD_USBPORT_DEMO)
    add_subdirectory(app/tools/demoUsbPort)
endif()

if(BUILD_DEPTHQUALITY_DEMO)
    add_subdirectory(app/tools/demoDepthQuality)
endif()

if(BUILD_DEPTHSYNCRGB_DEMO)
    add_subdirectory(app/demoDsyncC)
endif()

if(BUILD_CNN_DEMO)
    add_subdirectory(app/demoCnn)
endif()

if(BUILD_AUTOCALIB_DEMO)
    message("make sure the SDK version should >= 4.37 .....")

    add_subdirectory(app/demoAutoCalib)
endif()



################################################################################
# Compilation options 
################################################################################



################################################################################
# Include directories
################################################################################


################################################################################
# Link options 
################################################################################
	

################################################################################
# Link
################################################################################


