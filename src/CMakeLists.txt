set(SOURCES
    r3000.h
    r3000interpreter.h
    types.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_compile_options(${PROJECT_NAME} PUBLIC -fpermissive)

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt) # TODO: Remove this
#target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive)