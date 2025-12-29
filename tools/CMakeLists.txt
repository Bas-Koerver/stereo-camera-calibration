#cmake_minimum_required(VERSION 3.23...3.27)
#project(stereo-camera-calibration
#        LANGUAGES CXX
#        VERSION 0.1)
#
#set(CMAKE_CXX_STANDARD 23)

#find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio aruco)
#find_package(OpenCV COMPONENTS core highgui imgproc videoio imgcodecs calib3d objdetect aruco REQUIRED)
#find_package(OpenCV REQUIRED)
#
#find_package(Boost COMPONENTS program_options REQUIRED)
#add_compile_definitions(BOOST_BIND_GLOBAL_PLACEHOLDERS) ## needed to get rid of warning `#pragma message: The practice of declaring the Bind placeholders (_1, _2, ...)`
#
#find_package(CLI11 REQUIRED)

#include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(create-charuco-board create_charuco_board.cpp)

target_link_libraries(create-charuco-board PRIVATE ${OpenCV_LIBS} CLI11::CLI11)