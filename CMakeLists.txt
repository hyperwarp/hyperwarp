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

cmake_minimum_required(VERSION 3.10)

project(hyperwarp-metadata)

# TODO: turn debugging off once we have a stable code base
# CAUTION: setting this to RelWithDebInfo seems to break threads on GCC 9
set(CMAKE_BUILD_TYPE Debug)

include(GNUInstallDirs)

add_subdirectory(metadata-model bin/metadata-model)
add_subdirectory(metadata-foundationdb bin/metadata-foundationdb)
add_subdirectory(metadata bin/metadata)
add_subdirectory(samples bin/samples)
