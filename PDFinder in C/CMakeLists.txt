cmake_minimum_required(VERSION 3.31)
project(FileWarden C)

set(CMAKE_C_STANDARD 11)


add_executable(FileWarden WIN32 main.c app.rc
        resources.h)

set_property(TARGET FileWarden PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)



target_link_directories(FileWarden PRIVATE "./include")
target_link_libraries(FileWarden PRIVATE 
    libmupdf.lib libthirdparty.lib 
    comctl32 legacy_stdio_definitions.lib
    oldnames.lib)


