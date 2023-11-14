# File containing CMake macros

include("${sconf_SOURCE_DIR}/cmake/enable_asan.cmake")
include("${sconf_SOURCE_DIR}/cmake/enable_ubsan.cmake")
include("${sconf_SOURCE_DIR}/cmake/enable_coverage.cmake")
include("${sconf_SOURCE_DIR}/cmake/enable_check_clang_tidy.cmake")
include("${sconf_SOURCE_DIR}/cmake/enable_check_cppcheck.cmake")
