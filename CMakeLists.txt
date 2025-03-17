cmake_minimum_required(VERSION 3.10)
project(tiny_smart_pointer_unitest LANGUAGES CXX)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

set(cxx_standard 20)
add_compile_options(-Wall -g)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

aux_source_directory(unitest UNITEST_SOURCES)

add_executable(tiny_smart_pointer_unitest ${UNITEST_SOURCES})

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

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    target_compile_options(tiny_smart_pointer_unitest PRIVATE
        --coverage
        -fprofile-arcs
        -ftest-coverage
        -fno-inline
        -fno-inline-small-functions
        -fno-default-inline
        -fno-exceptions
    )

    target_link_options(tiny_smart_pointer_unitest PRIVATE --coverage)

    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters
            COMMAND $<TARGET_FILE:tiny_smart_pointer_unitest>
            COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture
            --output-file ${CMAKE_BINARY_DIR}/coverage.info
            --rc lcov_branch_coverage=1
            COMMAND ${LCOV_PATH} --remove ${CMAKE_BINARY_DIR}/coverage.info
            '*/unitest/*'
            '/usr/*'
            --output-file ${CMAKE_BINARY_DIR}/coverage.filtered.info
            --rc lcov_branch_coverage=1
            COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage.filtered.info
            --output-directory ${CMAKE_BINARY_DIR}/coverage_report
            --rc genhtml_branch_coverage=1
            --legend
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report..."
        )
    endif()
endif()