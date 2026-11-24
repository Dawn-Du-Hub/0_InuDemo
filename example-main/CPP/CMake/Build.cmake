option(BUILD_PARAMS_DEMO "Build with params demo." ON)
option(BUILD_SENSOR_DEMO "Build with sensor demo." ON)
option(BUILD_RGB_DEMO "Build with rgb demo." ON)
option(BUILD_DEPTH_DEMO "Build with depth demo." ON)
option(BUILD_DISPARITY_DEMO "Build with disparity demo." ON)
option(BUILD_2DEPTH_DEMO "Build with 2depth demo." ON)
option(BUILD_IR_DEMO "Build with IR demo." ON)
option(BUILD_RECONNECT_DEMO "Build with reconnect demo." ON)
option(BUILD_D2C_DEMO "Build with D2C demo." ON)
option(BUILD_3drpartyFilter "Build 3drpart filter." ON)
option(BUILD_DEPTH3PARTYFILTER_DEMO "Build with Depth3partyFilter demo." ON)
option(BUILD_PCL_DEMO "Build with PCL demo." ON)
option(BUILD_DEPTHPCL_DEMO "Build with DepthPCL demo." ON)
option(BUILD_IMU_DEMO "Build with IMU demo." ON)
option(BUILD_TEMPERA_DEMO "Build with Temperature demo." ON)
option(BUILD_FISHEYE_DEMO "Build with Fisheye demo." ON)
option(BUILD_FEATURE_DEMO "Build with Feature demo." ON)
option(BUILD_USBPORT_DEMO "Build with check USB port" ON)
option(BUILD_DEPTHQUALITY_DEMO "Build with check depth quality" ON)
option(BUILD_DEPTHSYNCRGB_DEMO "Build with depth sync rgb quality" ON)
option(BUILD_CNN_DEMO "Build with cnn " ON)
option(BUILD_AUTOCALIB_DEMO "Build with autocalibration" OFF)



add_definitions(-D_GE432_)
#add_definitions(-D_GE438_)

add_definitions(-D_SHAREDATA_)
set(CMAKE_BUILD_TYPE "Debug")  #Release  Debug
#set(CMAKE_BUILD_TYPE "Release")  #Release  Debug


if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(INU_STREAMS_ROOT "C:/Program Files/Inuitive/InuDev")
    SET(OpenCV_DIR "D:/1_work/4_OpenCV/opencv/build/x64/vc16/lib")
    SET(PCL_DIR "C:/Program Files/PCL 1.13.1")

endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET(INU_STREAMS_ROOT "/opt/Inuitive/InuDev")
    option(BUILD_stableCheck "stability Checking." ON)
endif()

SET(INU_LIBRARY_DIR ${INU_STREAMS_ROOT}/lib)
