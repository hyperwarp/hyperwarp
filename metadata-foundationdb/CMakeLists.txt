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

add_library(metadata-foundationdb MODULE metadata-foundationdb.c)
add_custom_command(TARGET metadata-foundationdb PRE_BUILD COMMAND astyle ARGS "${CMAKE_CURRENT_SOURCE_DIR}/*.c,*.h")
target_link_libraries(metadata-foundationdb fdb_c)
target_link_libraries(metadata-foundationdb m)
target_link_libraries(metadata-foundationdb rt)
target_link_libraries(metadata-foundationdb hyperwarp-metadata)
target_link_libraries(metadata-foundationdb hyperwarp-metadata-model)
target_include_directories(metadata-foundationdb PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )
set_target_properties(metadata-foundationdb PROPERTIES PREFIX "")
install(TARGETS metadata-foundationdb LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/hyperwarp)
