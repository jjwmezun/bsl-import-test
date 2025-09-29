cmake_minimum_required(VERSION 3.2)

project(BSLImportTest)
add_executable(BSLImportTest
    src/data.c
    src/main.c
    src/text.c
)
set_property(TARGET BSLImportTest PROPERTY C_STANDARD 23)
include_directories(include include)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})