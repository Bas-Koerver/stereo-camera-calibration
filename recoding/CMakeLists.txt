find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)

add_executable(prophesee-dvs-recorder prophesee_dvs_recorder.cpp)

target_link_libraries(prophesee-dvs-recorder PRIVATE
        MetavisionSDK::core MetavisionSDK::stream MetavisionSDK::ui)

set_target_properties(prophesee-dvs-recorder PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/run-mv")

# copy only Metavision DLLs
set(MV "C:/Program Files/Prophesee")

add_custom_command(TARGET prophesee-dvs-recorder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${MV}/bin" "$<TARGET_FILE_DIR:prophesee-dvs-recorder>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${MV}/third_party/$<IF:$<CONFIG:Debug>,debug/,>bin" "$<TARGET_FILE_DIR:prophesee-dvs-recorder>")