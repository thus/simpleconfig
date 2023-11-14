# File containing CMake macros

include("${PROJECT_SOURCE_DIR}/cmake/enable_asan.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/enable_ubsan.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/enable_coverage.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/enable_check_clang_tidy.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/enable_check_cppcheck.cmake")
