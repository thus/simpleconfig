cmake_minimum_required(VERSION 3.1)

project(
  sconf
  DESCRIPTION "A simpler aproach to config"
  VERSION 0.2
  LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

include("${PROJECT_SOURCE_DIR}/cmake/macros.cmake")

option(BUILD_SHARED "Build shared library" ON)
option(BUILD_STATIC "Build static library" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_FUZZERS "Build fuzzer applications" OFF)
option(ENABLE_TESTS "Enable tests" OFF)
option(ENABLE_COVERAGE "Enable coverage report" OFF)
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_CLANG_TIDY "Enable linter 'clang-tidy'" OFF)
option(ENABLE_CPPCHECK "Enable linter 'cppcheck'" OFF)

add_subdirectory(third_party)

if(ENABLE_CLANG_TIDY)
    enable_check_clang_tidy()
endif()

if(ENABLE_CPPCHECK)
    enable_check_cppcheck()
endif()

add_custom_target(check DEPENDS ${all_linters})

if(ENABLE_ASAN)
    enable_asan()
endif()

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(ENABLE_COVERAGE)
    enable_coverage()
endif()

if(ENABLE_TESTS)
    enable_testing()
    set(CMAKE_CTEST_ARGUMENTS --output-on-failure)
    add_subdirectory(tests)
endif()

if(BUILD_FUZZERS)
    add_subdirectory(fuzz)
endif()
