find_library(CMOCKA_LIB cmocka)

# Add tests to this list
set(SCONF_TESTS
    test_sconf_yaml_read
    test_sconf_type_to_str
    test_sconf_type_to_arg_type_str
    test_sconf_err_set
    test_sconf_set_and_get_str
    test_sconf_set_and_get_bool
    test_sconf_set_and_get_int
    test_sconf_set_and_get_float
    test_sconf_node_dict
    test_sconf_node_array
    test_sconf_node_dict_foreach
    test_sconf_node_array_foreach
    test_sconf_node_array_next
    test_sconf_opts_parse
    test_sconf_defaults
    test_sconf_validate
    test_sconf_env_read
    test_sconf_initialize
)

foreach(X IN LISTS SCONF_TESTS)
    add_executable(${X} ${X}.c)
    target_compile_options(${X} PRIVATE -Wall -Wno-missing-field-initializers -Wno-missing-braces)
    target_link_libraries(${X} sconf ${CMOCKA_LIB})
    add_test(NAME ${X} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${X}
             WORKING_DIRECTORY ${simpleconfig_SOURCE_DIR}/tests
    )
endforeach()
