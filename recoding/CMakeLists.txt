cmake_minimum_required(VERSION 3.23...3.27)
project(stereo-camera-calibration
LANGUAGES CXX
VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)

find_package(CLI11 REQUIRED)

find_package(OpenCV COMPONENTS aruco REQUIRED)

find_package(Boost COMPONENTS program_options REQUIRED)
add_compile_definitions(BOOST_BIND_GLOBAL_PLACEHOLDERS) ## needed to get rid of warning `#pragma message: The practice of declaring the Bind placeholders (_1, _2, ...)`

add_executable(prophesee-dvs-recorder prophesee_dvs_recorder.cpp)
target_link_libraries(prophesee-dvs-recorder PRIVATE
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
        ${OpenCV_LIBS})