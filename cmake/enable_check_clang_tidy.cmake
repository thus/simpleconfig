# Makes it possible to run 'make check-clang-tidy'
macro(enable_check_clang_tidy)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(compdb ${CMAKE_BINARY_DIR}/compile_commands.json)
    set(lint_dir ${CMAKE_SOURCE_DIR}/src/*.c)
    set(config_file ${CMAKE_SOURCE_DIR}/.clang-tidy)
    add_custom_target(check-clang-tidy clang-tidy -p ${compdb} --config-file=${config_file} ${lint_dir} 2>/dev/null)
    add_custom_target(check DEPENDS check-clang-tidy)
endmacro(enable_check_clang_tidy)#
