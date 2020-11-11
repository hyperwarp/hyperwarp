# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2020 Hyperwarp Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.h" "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.c"
                   COMMAND "protoc-c" ARGS "--c_out=${CMAKE_CURRENT_SOURCE_DIR}" "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}" "metadata.proto")

add_library(hyperwarp-metadata-model SHARED "${CMAKE_CURRENT_SOURCE_DIR}/metadata.pb-c.c")
target_link_libraries(hyperwarp-metadata-model protobuf-c)
target_include_directories(hyperwarp-metadata-model PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )

set_target_properties(hyperwarp-metadata-model PROPERTIES PUBLIC_HEADER metadata.pb-c.h)
install(TARGETS hyperwarp-metadata-model LIBRARY PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hyperwarp)