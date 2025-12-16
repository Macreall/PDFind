cmake_minimum_required(VERSION 3.31)
project(PDFinder C)

set(CMAKE_C_STANDARD 11)

include_directories("resources/SDL2/include")
link_directories("resources/SDL2/lib")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

add_executable(PDFinder main.c)

target_link_libraries(PDFinder SDL2 SDL2main opengl32)

set_target_properties(PDFinder PROPERTIES OUTPUT_NAME "PDFinder")
