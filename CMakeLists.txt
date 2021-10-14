cmake_minimum_required(VERSION 3.8)

project(Meeps)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add the module directory to the list of paths
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

option(MEEPS_TESTS "Build tests" OFF)
if (MEEPS_TESTS)
    message("Buliding tests!")
    # Enable unit-testing.
    enable_testing(true)
    find_package(Unicorn REQUIRED)
    add_subdirectory(tests)
endif()

add_subdirectory(src)
add_subdirectory(externals)