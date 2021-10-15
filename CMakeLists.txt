cmake_minimum_required(VERSION 3.8)

project(Meeps)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(MEEPS_TESTS "Build tests" OFF)
option(MEEPS_WARNINGS_AS_ERRORS "Warnings as errors" OFF)

# Add the module directory to the list of paths
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

# Compiler flags
if (MSVC)
    set(CXX_FLAGS
        /std:c++latest # CMAKE_CXX_STANDARD has no effect on MSVC until CMake 3.10.
        /experimental:external
        /external:W0
        /external:anglebrackets
        /W4
        /w44263 # Non-virtual member function hides base class virtual function
        /w44265 # Class has virtual functions, but destructor is not virtual
        /w44456 # Declaration of 'var' hides previous local declaration
        /w44457 # Declaration of 'var' hides function parameter
        /w44458 # Declaration of 'var' hides class member
        /w44459 # Declaration of 'var' hides global definition
        /w44946 # Reinterpret-cast between related types
        /wd4592 # Symbol will be dynamically initialized (implementation limitation)
        /permissive- # Stricter C++ standards conformance
        /MP
        /Zi
        /Zo
        /EHsc
        /Zc:externConstexpr # Allows external linkage for variables declared "extern constexpr", as the standard permits.
        /Zc:inline          # Omits inline functions from object-file output.
        /Zc:throwingNew     # Assumes new (without std::nothrow) never returns null.
        /volatile:iso       # Use strict standard-abiding volatile semantics
        /bigobj             # Increase number of sections in .obj files
        /DNOMINMAX)

    if (MEEPS_WARNINGS_AS_ERRORS)
        list(APPEND CXX_FLAGS
             /WX)
    endif()

    if (CMAKE_VS_PLATFORM_TOOLSET MATCHES "LLVM-vs[0-9]+")
        list(APPEND CXX_FLAGS
             -Qunused-arguments
             -Wno-missing-braces)
    endif()
else()
    set(CXX_FLAGS
        -Wall
        -Wextra
        -Wcast-qual
        -pedantic
        -pedantic-errors
        -Wno-missing-braces)

    if (MEEPS_WARNINGS_AS_ERRORS)
        list(APPEND CXX_FLAGS
             -Werror)
    endif()
endif()

if (MEEPS_TESTS)
    message("Buliding tests!")
    # Enable unit-testing.
    enable_testing(true)
    find_package(Unicorn REQUIRED)
    add_subdirectory(tests)
endif()

add_subdirectory(src)
add_subdirectory(externals)