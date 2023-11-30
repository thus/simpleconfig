include(FindPackageHandleStandardArgs)

find_library(
  YAML_LIBRARY
  NAMES yaml
)

find_path(
  YAML_INCLUDE_DIR
  NAMES yaml.h
  HINTS ${CMAKE_INSTALL_INCLUDEDIR}
)

find_package_handle_standard_args(yaml
  DEFAULT_MSG
  YAML_LIBRARY YAML_INCLUDE_DIR
)

mark_as_advanced(YAML_LIBRARY YAML_INCLUDE_DIR)

if(yaml_FOUND AND NOT TARGET yaml)
  add_library(yaml SHARED IMPORTED)

  set_target_properties(yaml
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${YAML_INCLUDE_DIR}
      IMPORTED_LOCATION ${YAML_LIBRARY}
  )
endif()
