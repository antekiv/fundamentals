set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER g++ CACHE STRING "C++ compiler" FORCE)

add_library(ProjectConfiguration INTERFACE)

# For GCC, Clang and AppleClang:
# -Wall: Enables all the common warning messages.
# -Wextra: Enables additional warning messages not covered by -Wall.
# -Wpedantic: Enforces strict ISO compliance.
# -Wconversion: Warns on type conversions that may alter a value.
# -Wsign-conversion: Warns on sign conversions.
# -Wshadow: Warns when a variable declaration shadows one from a parent context.
# -Werror: Treats all warnings as errors.

# For MSVC:
# /W4: Sets the warning level to 4, enabling most warnings.
# /WX: Treats all warnings as errors.
target_compile_options(ProjectConfiguration
    INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror
        -fsanitize=address
        -fsanitize=undefined>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX>
)
target_compile_features(ProjectConfiguration
    INTERFACE
        cxx_std_20
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(SANITIZE_FLAGS "-fsanitize=address,undefined")
    add_compile_options(${SANITIZE_FLAGS} -g)
    add_link_options(${SANITIZE_FLAGS})
endif()

# C++23
add_library(ProjectConfiguration23 INTERFACE)

target_compile_features(ProjectConfiguration23
    INTERFACE
        cxx_std_23
)

function(add_project_executable target_name)
    add_executable(${target_name} ${target_name}.cpp)

    target_link_libraries(${target_name}
        PRIVATE
            ProjectConfiguration23
    )
endfunction()


function(add_project_test_executable target_name)
    add_executable("test_${target_name}" "test_${target_name}.cpp" ${target_name}.cpp)

    target_link_libraries("test_${target_name}"
        PRIVATE
            ProjectConfiguration23
    )
endfunction()