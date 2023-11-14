include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/third_party")

# A minimal example showing off the basics of sconf
add_executable(minimal minimal.c)
target_link_libraries(minimal sconf)
