cmake_minimum_required(VERSION 4.0)
project(stereo-camera-calibration)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenCV 4.11 REQUIRED)


if (TARGET opencv_aruco OR OpenCV_aruco_FOUND)
    message(STATUS "ArUco module found!")
else ()
    message(STATUS "ArUco module not found! skipping Aruco calibration module")
endif ()

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(stereo-camera-calibration main.cpp)

target_link_libraries(stereo-camera-calibration ${OpenCV_LIBS})
