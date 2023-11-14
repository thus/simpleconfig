# libart
add_library(art STATIC art.c)
target_include_directories(art INTERFACE "${sconf_SOURCE_DIR}/third_party")
target_compile_options(art PRIVATE -Wall -fPIC)
