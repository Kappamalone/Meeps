set(SOURCES
    r3000.h
    r3000interpreter.h
    cop0.h
    types.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt)