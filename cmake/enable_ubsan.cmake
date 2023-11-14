# Enable undefined behaviour sanitizer
macro(enable_ubsan)
    add_compile_options(-fno-omit-frame-pointer -g -fsanitize=undefined)
    add_link_options(-fno-omit-frame-pointer -g -fsanitize=undefined)
endmacro(enable_ubsan)#
