add_library(Maths STATIC
    Ref/VecBatchRef.cpp
    Ref/MatBatchRef.cpp
    SSE/VecBatchSSE.cpp
    SSE/MatBatchSSE.cpp
    ASM/Vec3.asm
    ASM/Vec3.h
        SIMD/Vec3.h
)

target_include_directories(Maths PUBLIC
    ${CMAKE_SOURCE_DIR}
)

if (MSVC)
    set(SIMD_FLAGS /arch:AVX2)
else()
    set(SIMD_FLAGS -mavx2)
endif()

target_compile_options(Maths PUBLIC ${SIMD_FLAGS})
