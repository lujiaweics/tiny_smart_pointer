cmake_minimum_required(VERSION 3.10)
project(tiny_smart_pointer_unitest LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdparty/CMake-codecov/cmake" ${CMAKE_MODULE_PATH})

find_package(codecov)

option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Enabling code coverage instrumentation")
    
    # 设置覆盖率工具（lcov或gcovr）
    set(COVERAGE_TOOL "lcov" CACHE STRING "Coverage tool lcov")
    
    # 排除不需要统计的目录
    set(COVERAGE_EXCLUDES
        "${CMAKE_SOURCE_DIR}/unitest/*"
    )
    
    # 启用覆盖率检测
    enable_coverage(
        TARGETS tiny_smart_pointer_unitest  # 你的测试可执行文件
        TYPE ${COVERAGE_TOOL}
        EXCLUDE "${COVERAGE_EXCLUDES}"
        LCOV_ARGS "--rc lcov_branch_coverage=1"  # 启用分支覆盖率
        GCOVR_ARGS "--exclude-unreachable-branches"
    )
endif()

set(cxx_standard 20)
add_compile_options(-Wall)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(tiny_smart_pointer_unitest
    unitest/main.cpp
    unitest/test_sharedpointer.cpp
    unitest/test_weakpointer.cpp
)

target_include_directories(tiny_smart_pointer_unitest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tiny_smart_pointer_unitest PRIVATE
    GTest::GTest
    GTest::Main
    Threads::Threads
)

set_target_properties(tiny_smart_pointer_unitest PROPERTIES
    CXX_STANDARD ${cxx_standard}
    CXX_STANDARD_REQUIRED ON
)

enable_testing()
add_test(
    NAME smart_pointer_tests
    COMMAND $<TARGET_FILE:tiny_smart_pointer_unitest>
)