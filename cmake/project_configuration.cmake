set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX>
)
target_compile_features(ProjectConfiguration
    INTERFACE
        cxx_std_20
)
