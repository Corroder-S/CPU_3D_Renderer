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


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        src/*.cpp src/*.h src/*.inl
        Editor/*.cpp Editor/*.h
)

add_executable(cpu_renderer_sdl ${SRC_FILES})

target_link_libraries(cpu_renderer_sdl PRIVATE SDL3::SDL3 Maths nlohmann_json::nlohmann_json)

target_compile_options(cpu_renderer_sdl PRIVATE
    $<$<CONFIG:Release>:/O2 /fp:fast /arch:AVX2>
    $<$<CONFIG:Debug>:/Od /arch:AVX2>
)


target_include_directories(cpu_renderer_sdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(cpu_renderer_sdl PRIVATE
        ASSETS_PATH="Asset/"
)

add_custom_command(TARGET cpu_renderer_sdl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:cpu_renderer_sdl>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Asset
        $<TARGET_FILE_DIR:cpu_renderer_sdl>/Asset
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
