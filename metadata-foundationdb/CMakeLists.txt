add_library(hyperwarp-metadata-fdb SHARED metadata-foundationdb.h metadata-foundationdb.c)
target_link_libraries(hyperwarp-metadata-fdb fdb_c)
target_link_libraries(hyperwarp-metadata-fdb m)
target_link_libraries(hyperwarp-metadata-fdb rt)
target_link_libraries(hyperwarp-metadata-fdb hyperwarp-metadata-model)
target_include_directories(hyperwarp-metadata-fdb PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )