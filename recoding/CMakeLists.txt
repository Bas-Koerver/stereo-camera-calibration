#cmake_minimum_required(VERSION 3.23...3.27)
#project(stereo-camera-calibration
#        LANGUAGES CXX
#        VERSION 0.1)
#
#set(CMAKE_CXX_STANDARD 23)

#find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)
#find_package(pylon REQUIRED)
#
#find_package(CLI11 REQUIRED)
#
#find_package(nlohmann_json REQUIRED)
#
#find_package(OpenCV REQUIRED)
#
#find_package(Boost COMPONENTS program_options REQUIRED)
#add_compile_definitions(BOOST_BIND_GLOBAL_PLACEHOLDERS) ## needed to get rid of warning `#pragma message: The practice of declaring the Bind placeholders (_1, _2, ...)`
#
#
#include(FetchContent)
#FetchContent_Declare(
#        readerwriterqueue
#        GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue
#        GIT_TAG master
#)
#FetchContent_MakeAvailable(readerwriterqueue)
#
#
#include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(
        StereoCalibration
        manager.cpp
        recorders/camera_worker.cpp recorders/camera_worker.hpp
        recorders/prophesee_cam_worker.cpp recorders/prophesee_cam_worker.hpp
        recorders/basler_cam_worker.cpp recorders/basler_cam_worker.hpp
        Utility.cpp Utility.hpp
        Video_viewer.cpp Video_viewer.hpp
        detection_validator.cpp detection_validator.hpp
        Image_validator.cpp Image_validator.hpp
        calibration.cpp calibration.hpp
        job_data.hpp
)

target_link_libraries(
        StereoCalibration
        PRIVATE
        ${OpenCV_LIBS}
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
        pylon::pylon
        nlohmann_json::nlohmann_json
        PUBLIC
        readerwriterqueue
)

