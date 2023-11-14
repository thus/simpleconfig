cmake_minimum_required(VERSION 3.0)

project(
  sconf
  DESCRIPTION "A simpler aproach to config"
  VERSION 0.2
  LANGUAGES C
)

include("${PROJECT_SOURCE_DIR}/cmake/macros.cmake")

option(BUILD_SHARED "Build shared library" ON)
option(BUILD_STATIC "Build static library" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_TESTS "Enable tests" OFF)
option(ENABLE_COVERAGE "Enable coverage report" OFF)
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_LINTING "Enable linting" ON)

add_subdirectory(third_party)

if(ENABLE_LINTING)
    enable_check_clang_tidy()
    enable_check_cppcheck()
    add_custom_target(check DEPENDS ${all_linters})
endif()

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

