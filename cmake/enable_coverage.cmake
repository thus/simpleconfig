# Makes it possible to run 'make coverage'
macro(enable_coverage)
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(GCOV_TOOL --gcov-tool ${PROJECT_SOURCE_DIR}/tools/llvm-gcov.sh)
    endif()

    set(LCOV_CMD lcov -c -d ./src -o cov.info ${GCOV_TOOL})
    set(GENHTML_CMD genhtml cov.info -o coverage)
    set(PRINT_LINK_CMD echo LINK: file://`pwd`/coverage/index.html)

    add_custom_target(coverage COMMAND ${LCOV_CMD} && ${GENHTML_CMD} && ${PRINT_LINK_CMD})
endmacro(enable_coverage)#
