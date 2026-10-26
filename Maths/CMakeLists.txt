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
