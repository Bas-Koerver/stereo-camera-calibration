find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio aruco)

find_package(CLI11 REQUIRED)

add_executable(create-charuco-board create_charuco_board.cpp)

target_link_libraries(create-charuco-board PRIVATE ${OpenCV_LIBS} CLI11::CLI11)

set_target_properties(create-charuco-board PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/run-ocv")

add_custom_command(TARGET create-charuco-board POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<IF:$<CONFIG:Debug>,debug/,>bin"
        "$<TARGET_FILE_DIR:create-charuco-board>")
