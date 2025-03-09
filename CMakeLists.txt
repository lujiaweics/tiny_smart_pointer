cmake_minimum_required(VERSION 3.10)
project(tiny_smart_pointer_unitest LANGUAGES CXX)

set(cxx_standard 20)
add_compile_options(-Wall)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(tiny_smart_pointer_unitest
    unitest/main.cpp
    unitest/test_sharedpointer.cpp
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