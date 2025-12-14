cmake_minimum_required(VERSION 3.31)
project(SDL_App C)

set(CMAKE_C_STANDARD 11)

include_directories("resources/SDL2/include")
link_directories("resources/SDL2/lib")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")


add_executable(SDL_App main.c)

target_link_libraries(SDL_App SDL2 SDL2main opengl32)
