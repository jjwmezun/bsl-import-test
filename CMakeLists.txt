cmake_minimum_required(VERSION 3.2)

project(BSLImportTest)
set(OpenGL_GL_PREFERENCE "GLVND")
add_executable(BSLImportTest
    src/data.c
    src/main.c
    src/matrix.c
    src/text.c
)
set_property(TARGET BSLImportTest PROPERTY C_STANDARD 23)
target_link_libraries(BSLImportTest m)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
find_path(GLEW_INCLUDE_DIR GL/glew.h)
find_path(GLU_INCLUDE_DIR GL/glu.h)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(include include)
target_link_libraries(BSLImportTest brotlienc brotlidec brotlicommon)
target_link_libraries(BSLImportTest ${OPENGL_LIBRARIES})
target_link_libraries(BSLImportTest ${GLEW_LIBRARIES})
target_link_libraries(BSLImportTest SDL2)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})