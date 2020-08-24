
# add the executable
add_executable(physical-disk-sample physical-disk-sample.c)

target_link_libraries(physical-disk-sample fdb_c)
target_link_libraries(physical-disk-sample m)
target_link_libraries(physical-disk-sample pthread)
target_link_libraries(physical-disk-sample rt)
target_link_libraries(physical-disk-sample hyperwarp-metadata-fdb)

target_include_directories(physical-disk-sample PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )