cmake_minimum_required(VERSION 3.19)
project(fundametals)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(project_configuration)

add_executable(string_test main.cpp)
target_link_libraries(string_test
    PRIVATE
        ProjectConfiguration
)
