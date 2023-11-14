# Add fuzzers to this list
set(SCONF_FUZZERS
    fuzz_sconf_get_set
    fuzz_sconf_yaml_read
)

foreach(name IN LISTS SCONF_FUZZERS)
    add_executable(${name} ${name}.c)
    target_link_libraries(${name} PRIVATE sconf)
endforeach()
