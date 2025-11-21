cmake_minimum_required(VERSION 3.23...3.27)
project(stereo-camera-calibration
LANGUAGES CXX
VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)
find_package(pylon REQUIRED)

find_package(CLI11 REQUIRED)

find_package(OpenCV REQUIRED)

find_package(Boost COMPONENTS program_options REQUIRED)
add_compile_definitions(BOOST_BIND_GLOBAL_PLACEHOLDERS) ## needed to get rid of warning `#pragma message: The practice of declaring the Bind placeholders (_1, _2, ...)`

add_executable(
        prophesee-dvs-recorder
        prophesee_dvs_recorder.cpp
        utility.cpp utility.h
)

add_executable(
        basler-rgb-recorder
        basler_rgb_recorder.cpp
        utility.cpp utility.h
)

add_executable(
        check-valid-images
        check_valid_images.cpp
)

target_link_libraries(prophesee-dvs-recorder PRIVATE
        ${OpenCV_LIBS}
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
)

target_link_libraries(basler-rgb-recorder PRIVATE
        ${OpenCV_LIBS}
        pylon::pylon
)

target_link_libraries(check-valid-images PRIVATE
        ${OpenCV_LIBS}
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
)