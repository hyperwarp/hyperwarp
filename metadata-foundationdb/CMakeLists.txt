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

add_library(hyperwarp-metadata-fdb SHARED metadata-foundationdb.c)
add_custom_command(TARGET hyperwarp-metadata-fdb PRE_BUILD COMMAND astyle ARGS "${CMAKE_CURRENT_SOURCE_DIR}/*.c,*.h")
target_link_libraries(hyperwarp-metadata-fdb fdb_c)
target_link_libraries(hyperwarp-metadata-fdb m)
target_link_libraries(hyperwarp-metadata-fdb rt)
target_link_libraries(hyperwarp-metadata-fdb hyperwarp-metadata-model)
target_include_directories(hyperwarp-metadata-fdb PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )

set_target_properties(hyperwarp-metadata-fdb PROPERTIES PUBLIC_HEADER metadata-foundationdb.h)
install(TARGETS hyperwarp-metadata-fdb LIBRARY PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hyperwarp)
