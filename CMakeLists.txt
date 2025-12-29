cmake_minimum_required(VERSION 3.23...3.27)
project(stereo-camera-calibration
        LANGUAGES CXX
        VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)
find_package(pylon REQUIRED)

find_package(CLI11 REQUIRED)

find_package(nlohmann_json REQUIRED)

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

add_subdirectory(recoding)
add_subdirectory(tools)