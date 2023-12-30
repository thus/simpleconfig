include(FindPackageHandleStandardArgs)

find_library(
  CMOCKA_LIBRARY
  NAMES cmocka
)

find_path(
  CMOCKA_INCLUDE_DIR
  NAMES cmocka.h
  HINTS ${CMAKE_INSTALL_INCLUDEDIR}
)

find_package_handle_standard_args(cmocka
  DEFAULT_MSG
  CMOCKA_LIBRARY CMOCKA_INCLUDE_DIR
)

mark_as_advanced(CMOCKA_LIBRARY CMOCKA_INCLUDE_DIR)

if(cmocka_FOUND AND NOT TARGET cmocka)
  add_library(cmocka::cmocka SHARED IMPORTED)

  set_target_properties(cmocka::cmocka
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${CMOCKA_INCLUDE_DIR}
      IMPORTED_LOCATION ${CMOCKA_LIBRARY}
  )
endif()
