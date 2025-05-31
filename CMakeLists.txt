cmake_minimum_required(VERSION 3.19)
project(SimpleCMakeProject)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(project_configuration)

add_executable(simple_cmake_project main.cpp)
target_link_libraries(simple_cmake_project
    PRIVATE
        ProjectConfiguration
)
