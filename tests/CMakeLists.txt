

add_executable(MathStarterTests
    Vec3Tests.cpp
    Vec4Tests.cpp
    Matrix3x3Tests.cpp
    Matrix4x4Tests.cpp
    VecBatchTest.cpp
    MatBatchTest.cpp
    ColorTests.cpp
    ClippingTests.cpp
    ASM_Vec3Tests.cpp
    ShaderTests.cpp
    TransformTests.cpp 
    MVPTests.cpp)

target_link_libraries(MathStarterTests PRIVATE Maths GTest::gtest_main)

target_include_directories(MathStarterTests PRIVATE 
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GoogleTest)
gtest_discover_tests(MathStarterTests)