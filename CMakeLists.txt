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

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)


add_subdirectory(Maths)


add_executable(cpu_renderer_sdl
        Editor/main.cpp
        Editor/Application.cpp
        Editor/Application.h
        src/Parsing/ObjLoader.cpp
        src/Parsing/MtlLoader.cpp
        src/Platform/MemoryMappedFile.h
        src/Platform/MemoryMappedFile.cpp
        src/Asset/Mesh.h
        src/Asset/Texture.cpp
        src/Asset/Light.h
        src/Asset/Mesh.cpp
        src/Renderer/clip.h
        src/Engine/Engine.cpp
        src/Engine/Engine.h
        src/Engine/Time.cpp
        src/Engine/Time.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Asset/Material.h
        src/Asset/Color.h
        src/Renderer/Shader.h
        src/Parsing/MtlLoader.h
        src/Asset/Font.h
        src/Asset/Font.cpp
        src/Renderer/Emitter/EmitterConfig.h
        src/Renderer/Emitter/EmitterConfig.cpp
        src/Renderer/Emitter/Particle.h
        src/Renderer/Emitter/Particle.cpp
        src/Renderer/Emitter/particleEmitter.h
        src/Renderer/Emitter/particleEmitter.cpp
        src/Renderer/Emitter/ParticleSystem.h
        src/Renderer/Emitter/ParticleSystem.cpp
        src/Renderer/Emitter/ParticleMemoryPools.h
        src/Renderer/Emitter/ParticleMemoryPools.cpp
        src/Renderer/Emitter/ParticleMemoryManager.h
        src/Renderer/Emitter/ParticleMemoryManager.cpp
        src/Renderer/Emitter/ParticleUpdateJob.h
        src/Renderer/Emitter/ParticleUpdateJob.cpp
        src/Renderer/Emitter/MasterParticleVertexBuffer.h
        src/Renderer/Emitter/MasterParticleVertexBuffer.cpp
        src/Renderer/Emitter/ParticleManager.h
        src/Renderer/Emitter/ParticleManager.cpp)

target_link_libraries(cpu_renderer_sdl PRIVATE SDL3::SDL3 Maths nlohmann_json::nlohmann_json)

target_include_directories(cpu_renderer_sdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(cpu_renderer_sdl PRIVATE
        ASSETS_PATH="${CMAKE_SOURCE_DIR}/Asset/"
)

add_custom_command(TARGET cpu_renderer_sdl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:cpu_renderer_sdl>
)

option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()

    # Téléchargement automatique de GoogleTest
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # Empêche GoogleTest de surcharger le runtime MSVC sous Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Inclusion du sous-dossier de tests
    add_subdirectory(tests)
endif()
