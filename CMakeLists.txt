cmake_minimum_required(VERSION 4.2)
project(snake_sdl3)

set(CMAKE_CXX_STANDARD 26)

include_directories(include)
link_directories(lib)

add_executable(snake_sdl3 src/main.cpp)

if (WIN32)
    target_link_libraries(snake_sdl3 SDL3.lib SDL3_image.lib)
endif ()