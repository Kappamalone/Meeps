set(TESTS "${PROJECT_NAME}_tests")
set(SOURCES
    test_memory.h
    unicorn_emu.h
    test_instructions.cpp
    test_memory.cpp
    test_main.cpp
)

add_executable(${TESTS} ${SOURCES})
target_link_libraries(${TESTS} PRIVATE ${PROJECT_NAME} doctest fmt Unicorn::Unicorn)

add_test(NAME ${TESTS} COMMAND ${TESTS}) # what does this do exactly?