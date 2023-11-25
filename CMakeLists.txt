cmake_minimum_required(VERSION 3.1)

project(
  simpleconfig
  DESCRIPTION "A simpler approach to configuration handling in projects"
  VERSION 1.0.1
  LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

include("${simpleconfig_SOURCE_DIR}/cmake/macros.cmake")

option(SCONF_BUILD_SHARED "Build shared library" ON)
option(SCONF_BUILD_STATIC "Build static library" ON)
option(SCONF_BUILD_EXAMPLES "Build examples" OFF)
option(SCONF_BUILD_FUZZERS "Build fuzzer applications" OFF)
option(SCONF_ENABLE_TESTS "Enable tests" OFF)
option(SCONF_ENABLE_COVERAGE "Enable coverage report" OFF)
option(SCONF_ENABLE_ASAN "Enable address sanitizer" OFF)
option(SCONF_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)
option(SCONF_ENABLE_CLANG_TIDY "Enable linter 'clang-tidy'" OFF)
option(SCONF_ENABLE_CPPCHECK "Enable linter 'cppcheck'" OFF)

add_subdirectory(third_party)

if(SCONF_ENABLE_CLANG_TIDY)
    enable_check_clang_tidy()
endif()

if(SCONF_ENABLE_CPPCHECK)
    enable_check_cppcheck()
endif()

if(all_linters)
    add_custom_target(check DEPENDS ${all_linters})
endif()

if(SCONF_ENABLE_ASAN)
    enable_asan()
endif()

if(SCONF_ENABLE_UBSAN)
    enable_ubsan()
endif()

add_subdirectory(src)

if(SCONF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(SCONF_ENABLE_COVERAGE)
    enable_coverage()
endif()

if(SCONF_ENABLE_TESTS)
    enable_testing()
    set(CMAKE_CTEST_ARGUMENTS --output-on-failure)
    add_subdirectory(tests)
endif()

if(SCONF_BUILD_FUZZERS)
    add_subdirectory(fuzz)
endif()
