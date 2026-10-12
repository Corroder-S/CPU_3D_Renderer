cmake_minimum_required(VERSION 3.20)
project(CpuEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(SDL3)

add_executable(cpu_renderer_sdl src/main.cpp)

target_link_libraries(cpu_renderer_sdl PRIVATE SDL3::SDL3)

target_include_directories(cpu_renderer_sdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)