add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.h" "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.c"
                   COMMAND "protoc-c" ARGS "--c_out=." "metadata.proto")

add_library(hyperwarp-metadata-model SHARED "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.h" "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.c")
target_link_libraries(hyperwarp-metadata-model protobuf-c)
target_include_directories(hyperwarp-metadata-model PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )