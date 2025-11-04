cmake_minimum_required(VERSION 4.0)
project(stereo-camera-calibration)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(recoding)
add_subdirectory(tools)



#find_package(OpenCV COMPONENTS core highgui imgproc videoio imgcodecs calib3d objdetect REQUIRED)
#if (TARGET opencv_aruco OR OpenCV_aruco_FOUND)
#    message(STATUS "ArUco module found!")
#else ()
#    message(STATUS "ArUco module not found! skipping Aruco calibration module")
#endif ()
#
#find_package(pylon REQUIRED)
#find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)
#find_package(CLI11 REQUIRED)
#
#include_directories(${OpenCV_INCLUDE_DIRS})
#
#add_executable(create-charuco-board tools/create_charuco_board.cpp)
#add_executable(prophesee-dvs-recorder recoding/prophesee_dvs_recorder.cpp)
#
#target_link_libraries(create-charuco-board PRIVATE
#        ${OpenCV_LIBS}
#        pylon::pylon
#        CLI11::CLI11
#)
#
#target_link_libraries(prophesee-dvs-recorder PRIVATE
#        MetavisionSDK::core
#        MetavisionSDK::stream
#        MetavisionSDK::ui
#        )