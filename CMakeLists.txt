cmake_minimum_required(VERSION 3.16)

project(SemiconductorController LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_library(device_controller
    ${PROJECT_SOURCE_DIR}/DeviceController.cpp
    ${PROJECT_SOURCE_DIR}/MockMotor.cpp
    ${PROJECT_SOURCE_DIR}/EquipmentController.cpp
)

target_include_directories(device_controller
    PUBLIC
        ${PROJECT_INCLUDE_DIR}
)

add_executable(controller_app
    ${PROJECT_SOURCE_DIR}/main.cpp
)

target_link_libraries(controller_app PRIVATE device_controller)

# Tests configuration
add_executable(device_controller_tests
    ${PROJECT_TEST_DIR}/test_device_controller.cpp
)

target_link_libraries(device_controller_tests PRIVATE device_controller)

enable_testing()
add_test(NAME device_controller_tests COMMAND device_controller_tests)
