include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/third_party")

if (ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "-O0 --coverage ${CMAKE_C_FLAGS}")
endif()

set(sconf_source
    array.c
    convert.c
    defaults.c
    env.c
    opts.c
    sconf.c
    validate.c
    yaml.c
)

set(sconf_compile_options -Wall -Wextra)

if (BUILD_SHARED)
    add_library(sconf SHARED ${sconf_source})
    target_compile_options(sconf PRIVATE ${sconf_compile_options})
    target_link_libraries(sconf art)
    target_link_libraries(sconf yaml)
    install(TARGETS sconf DESTINATION lib)
endif()

if (BUILD_STATIC)
    add_library(sconf_static STATIC ${sconf_source})
    target_compile_options(sconf_static PRIVATE ${sconf_compile_options})
    set_target_properties(sconf_static PROPERTIES OUTPUT_NAME sconf)
    target_link_libraries(sconf_static art)
    target_link_libraries(sconf_static yaml)
    install(TARGETS sconf_static DESTINATION lib)
endif()

install(FILES "${PROJECT_SOURCE_DIR}/include/sconf.h" DESTINATION include)
