# Enable address sanitizer
macro(enable_asan)
    add_compile_options(-fno-omit-frame-pointer -g -fsanitize=address)
    add_link_options(-fno-omit-frame-pointer -g -fsanitize=address)
endmacro(enable_asan)#
