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

macro(add_sample target source)
    add_executable(${target} ${source} util.h util.c)
    target_link_libraries(${target} pthread)
    target_link_libraries(${target} hyperwarp-metadata)
    target_link_libraries(${target} hyperwarp-metadata-fdb)
    target_include_directories(${target} PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}"
                            "${CMAKE_CURRENT_SOURCE_DIR}"
                            )
endmacro()

add_sample(allocation allocation.c)
add_sample(metadata-sample metadata-sample.c)
add_sample(physical-disk-sample physical-disk-sample.c)
