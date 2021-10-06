cmake_minimum_required(VERSION 3.8)

project(Meeps)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(MEEPS_TESTS "Build tests" OFF)
if (MEEPS_TESTS)
    message("Buliding tests!")
    # Enable unit-testing.
    enable_testing(true)
    add_subdirectory(tests)
endif()

add_subdirectory(src)
add_subdirectory(externals)