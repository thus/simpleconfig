# Makes it possible to run 'make check-cppcheck'
macro(enable_check_cppcheck)
    add_custom_target(check-cppcheck cppcheck --quiet --error-exitcode=1 ${PROJECT_SOURCE_DIR}/src/*.c)
    list(APPEND all_linters check-cppcheck)
endmacro(enable_check_cppcheck)#
