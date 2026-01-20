cmake_minimum_required(VERSION 3.23...3.27)
project(YACCP
        LANGUAGES CXX
        VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)
find_package(pylon REQUIRED)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(OpenCV REQUIRED)

find_package(nlohmann_json REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

find_package(CLI11 CONFIG REQUIRED)
find_package(tabulate CONFIG REQUIRED)

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
        YACCP

        src/yaccp.cpp

        src/utility.cpp src/utility.hpp
        src/camera_calibration.cpp src/camera_calibration.hpp

        src/recoding/detection_validator.cpp src/recoding/detection_validator.hpp
        src/recoding/video_viewer.cpp src/recoding/video_viewer.hpp
        src/recoding/job_data.hpp

        src/recoding/recorders/camera_worker.cpp src/recoding/recorders/camera_worker.hpp
        src/recoding/recorders/prophesee_cam_worker.cpp src/recoding/recorders/prophesee_cam_worker.hpp
        src/recoding/recorders/basler_cam_worker.cpp src/recoding/recorders/basler_cam_worker.hpp

        src/tools/create_board.cpp src/tools/create_board.hpp
        src/tools/image_validator.cpp src/tools/image_validator.hpp

        src/cli/orchestrator.cpp src/cli/orchestrator.hpp
        src/cli/board_creation.cpp src/cli/board_creation.hpp
        src/cli/calibration.cpp src/cli/calibration.hpp
        src/cli/recording.cpp src/cli/recording.hpp
        src/cli/validation.cpp src/cli/validation.hpp

        src/config/orchestrator.cpp src/config/orchestrator.hpp
        src/config/board.cpp src/config/board.hpp
        src/config/detection.cpp src/config/detection.hpp
        src/config/recording.cpp src/config/recording.hpp
        src/config/viewing.cpp src/config/viewing.hpp

        src/global_variables/config_defaults.hpp
        src/global_variables/cli_defaults.hpp
)

target_link_libraries(
        YACCP
        PRIVATE
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui
        pylon::pylon
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
        tomlplusplus::tomlplusplus
        CLI11::CLI11
        tabulate::tabulate
        PUBLIC
        readerwriterqueue
)