# File containing CMake macros

include("${simpleconfig_SOURCE_DIR}/cmake/enable_asan.cmake")
include("${simpleconfig_SOURCE_DIR}/cmake/enable_ubsan.cmake")
include("${simpleconfig_SOURCE_DIR}/cmake/enable_coverage.cmake")
include("${simpleconfig_SOURCE_DIR}/cmake/enable_check_clang_tidy.cmake")
include("${simpleconfig_SOURCE_DIR}/cmake/enable_check_cppcheck.cmake")
