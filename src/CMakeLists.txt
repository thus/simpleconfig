if (SCONF_ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "-O0 --coverage ${CMAKE_C_FLAGS}")
endif()

set(simpleconfig_source
    array.c
    convert.c
    defaults.c
    env.c
    opts.c
    sconf.c
    validate.c
    yaml.c
)

set(simpleconfig_compile_options -Wall -Wextra -Wno-missing-field-initializers -Wno-missing-braces)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(simpleconfig_compile_options ${simpleconfig_compile_options} -Werror)
endif()

find_package(yaml REQUIRED)

if (SCONF_BUILD_SHARED)
    add_library(sconf SHARED ${simpleconfig_source})
    target_include_directories(sconf PUBLIC "${simpleconfig_SOURCE_DIR}/include")
    target_compile_options(sconf PRIVATE ${simpleconfig_compile_options})
    target_link_libraries(sconf art)
    target_link_libraries(sconf yaml)
    install(TARGETS sconf DESTINATION lib)
endif()

if (SCONF_BUILD_STATIC)
    add_library(sconf_static STATIC ${simpleconfig_source})
    target_include_directories(sconf_static PUBLIC "${simpleconfig_SOURCE_DIR}/include")
    target_compile_options(sconf_static PRIVATE ${simpleconfig_compile_options})
    set_target_properties(sconf_static PROPERTIES OUTPUT_NAME sconf)
    target_link_libraries(sconf_static art)
    target_link_libraries(sconf_static yaml)
    install(TARGETS sconf_static DESTINATION lib)
endif()

install(FILES "${simpleconfig_SOURCE_DIR}/include/sconf.h" DESTINATION include)
