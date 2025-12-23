cmake_minimum_required(VERSION 3.23...3.27)
project(stereo-camera-calibration
        LANGUAGES CXX
        VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

#find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio aruco)
find_package(OpenCV COMPONENTS core highgui imgproc videoio imgcodecs calib3d objdetect aruco REQUIRED)

find_package(CLI11 REQUIRED)

add_executable(create-charuco-board create_charuco_board.cpp)

target_link_libraries(create-charuco-board PRIVATE ${OpenCV_LIBS} CLI11::CLI11)