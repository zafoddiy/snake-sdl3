cmake_minimum_required(VERSION 4.2)
project(snake_sdl3)

set(CMAKE_CXX_STANDARD 26)

include_directories(include)
link_directories(lib)
link_directories(assets/images)

add_executable(snake_sdl3 src/main.cpp)

if (WIN32)
    target_link_libraries(snake_sdl3 SDL3.lib SDL3_ttf.lib)
    add_custom_target(copy_resources ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets
            ${PROJECT_BINARY_DIR}/assets
            COMMENT "Copying assets into binary directory"
    )
    add_dependencies(snake_sdl3 copy_resources)
endif ()