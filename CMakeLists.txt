cmake_minimum_required(VERSION 3.20)
project(CpuEngine VERSION 1.0 LANGUAGES CXX ASM_MASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        main
)
FetchContent_MakeAvailable(SDL3)

add_subdirectory(Maths)


add_executable(cpu_renderer_sdl src/main.cpp
        src/Parsing/ObjLoader.cpp
        src/Platform/MemoryMappedFile.h
        src/Platform/MemoryMappedFile.cpp
        src/Asset/Mesh.h
        src/Asset/Texture.cpp
        src/Asset/Light.h
        src/Asset/Mesh.cpp "src/Renderer/clip.h")

target_link_libraries(cpu_renderer_sdl PRIVATE SDL3::SDL3 Maths)

target_include_directories(cpu_renderer_sdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(cpu_renderer_sdl PRIVATE
        ASSETS_PATH="${CMAKE_SOURCE_DIR}/Asset/"
)

add_custom_command(TARGET cpu_renderer_sdl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:cpu_renderer_sdl>
)
