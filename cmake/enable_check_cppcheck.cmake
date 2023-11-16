# Makes it possible to run 'make check-cppcheck'
macro(enable_check_cppcheck)
    find_program(cppcheck cppcheck)
    if (NOT cppcheck)
        message(FATAL_ERROR "Could not find linter 'cppcheck'")
    endif()

    add_custom_target(check-cppcheck ${cppcheck} --quiet --error-exitcode=1 ${simpleconfig_SOURCE_DIR}/src/*.c)
    list(APPEND all_linters check-cppcheck)
endmacro(enable_check_cppcheck)#
