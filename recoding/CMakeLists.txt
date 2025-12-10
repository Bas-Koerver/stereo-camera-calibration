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


include(FetchContent)
FetchContent_Declare(
        readerwriterqueue
        GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue
        GIT_TAG master
)
FetchContent_MakeAvailable(readerwriterqueue)


include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(
        StereoCalibration
        camera_manager.cpp
        recorders/camera_worker.cpp recorders/camera_worker.hpp
        recorders/prophesee_dvs_recorder.cpp recorders/prophesee_dvs_recorder.hpp
        recorders/basler_rgb_recorder.cpp recorders/basler_rgb_recorder.hpp
        utility.cpp utility.hpp
        videoViewer.cpp VideoViewer.hpp
        #        detection_validator.cpp                 detection_validator.h
)

target_link_libraries(
        StereoCalibration
        PRIVATE
        ${OpenCV_LIBS}
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
        pylon::pylon
        PUBLIC
        readerwriterqueue
)

#add_executable(
#        basler-rgb-recorder
#        basler_rgb_recorder.cpp
#        utility.cpp utility.h
#)

#add_executable(
#        metavision-filtering
#        metavision_filtering.cpp
#)

#add_executable(
#        check-valid-images
#        check_valid_images.cpp
#)

#target_link_libraries(basler-rgb-recorder PRIVATE
#        ${OpenCV_LIBS}
#        pylon::pylon
#)
#
#target_link_libraries(metavision-filtering PRIVATE
#        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
#        Boost::program_options
#)

#target_link_libraries(check-valid-images PRIVATE
#        ${OpenCV_LIBS}
#        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
#)

