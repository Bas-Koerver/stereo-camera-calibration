cmake_minimum_required(VERSION 4.0)
project(stereo-camera-calibration)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenCV REQUIRED)
if (TARGET opencv_aruco OR OpenCV_aruco_FOUND)
    message(STATUS "ArUco module found!")
else ()
    message(STATUS "ArUco module not found! skipping Aruco calibration module")
endif ()

find_package(pylon REQUIRED)
find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(stereo-camera-calibration calibration.cpp)
add_executable(prophesee-dvs-recorder prophesee_dvs_recorder.cpp)

target_link_libraries(stereo-camera-calibration
        ${OpenCV_LIBS}
        pylon::pylon
)

target_link_libraries(prophesee-dvs-recorder
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
        )