cmake_minimum_required(VERSION 3.10)

# set the project name
project(hyperwarp-metadata)

add_subdirectory(metadata-model bin/metadata-model)
add_subdirectory(metadata-foundationdb bin/metadata-foundationdb)
add_subdirectory(samples bin/samples)