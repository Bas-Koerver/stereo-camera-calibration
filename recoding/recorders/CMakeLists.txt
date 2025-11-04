cmake_minimum_required(VERSION 4.0)
project(prophesee-dvs-recorder)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} ../prophesee_dvs_recorder.cpp)

target_link_libraries(prophesee-dvs-recorder PRIVATE
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)