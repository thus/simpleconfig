# Makes it possible to run 'make coverage'
macro(enable_coverage)
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(gcov_tool --gcov-tool ${sconf_SOURCE_DIR}/tools/llvm-gcov.sh)
    endif()

    set(lcov_cmd lcov -c -d ./src -o cov.info ${gcov_tool})
    set(genhtml_cmd genhtml cov.info -o coverage)
    set(print_link_cmd echo LINK: file://`pwd`/coverage/index.html)
    add_custom_target(coverage COMMAND ${lcov_cmd} && ${genhtml_cmd} && ${print_link_cmd})
endmacro(enable_coverage)#
