cmake_minimum_required(VERSION 3.19)
project(fundametals)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(project_configuration)

add_executable(string_test main.cpp)
target_link_libraries(string_test
    PRIVATE
        ProjectConfiguration
)

add_executable(test_oop oop.cpp)
target_link_libraries(test_oop
    PRIVATE
        ProjectConfiguration
)


add_executable(vector vector.h 
                      test_vector.cpp)
target_link_libraries(vector
    PRIVATE
        ProjectConfiguration
)

add_project_executable(polimorphism)
add_project_executable(rtti)
add_project_executable(stack_allocator)
add_project_executable(move_semantics)
add_project_executable(type_deduction)
add_project_executable(tuple)
add_project_executable(unique_ptr)
add_project_executable(shared_ptr)
add_project_executable(any)