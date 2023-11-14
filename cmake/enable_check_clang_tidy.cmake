# Makes it possible to run 'make check-clang-tidy'
macro(enable_check_clang_tidy)
    find_program(clang_tidy clang-tidy)
    if (NOT clang_tidy)
        message(FATAL_ERROR "Could not find linter 'clang-tidy'")
    endif()

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(compdb ${CMAKE_BINARY_DIR}/compile_commands.json)
    set(lint_dir ${sconf_SOURCE_DIR}/src/*.c)
    set(config_file ${sconf_SOURCE_DIR}/.clang-tidy)

    add_custom_target(check-clang-tidy ${clang_tidy} -p ${compdb} --config-file=${config_file} ${lint_dir} 2>/dev/null)
    list(APPEND all_linters check-clang-tidy)
endmacro(enable_check_clang_tidy)#
