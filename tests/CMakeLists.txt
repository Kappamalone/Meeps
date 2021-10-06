set(TESTS "${PROJECT_NAME}_tests")
set(SOURCES
    test_memory.cpp
    test_main.cpp
)

add_executable(${TESTS} ${SOURCES})
target_link_libraries(${TESTS} PRIVATE ${PROJECT_NAME} doctest unicorn fmt)
#target_compile_options(${TESTS} PRIVATE -fpermissive)

add_test(${TESTS} ${TESTS} --duration yes)