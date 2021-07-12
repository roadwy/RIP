#
# Copyright (c) 2021.  https://github.com/geemion
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
#
cmake_minimum_required(VERSION 3.10)
project(beacon)

set(target_name beacon)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (LINUX TRUE)
endif()

add_definitions(-DYASIO_HEADER_ONLY)

set(VcpkgRoot ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})
set (BEACON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (PROTOBUF_CPP_DIR ${BEACON_SRC_DIR}/../proto_autogen_cpp)


set (BEACON_COMM_CORE ${BEACON_SRC_DIR}/main.cpp
    ${BEACON_SRC_DIR}/body/beacon/beacon.cpp
    ${BEACON_SRC_DIR}/body/netio/netio.cpp
    ${BEACON_SRC_DIR}/body/msghander/msg.cpp
    ${BEACON_SRC_DIR}/body/msghander/queue.cpp
    ${BEACON_SRC_DIR}/body/msghander/msghandler.cpp
    ${BEACON_SRC_DIR}/base/crypto/chacha20.cpp
    ${BEACON_SRC_DIR}/base/crypto/rsa.cpp
    ${BEACON_SRC_DIR}/base/log/log.cpp
        )

set (BEACON_WIN_SRC ${BEACON_SRC_DIR}/body/cmdexec/cmdexec_win.cpp
                    ${BEACON_SRC_DIR}/base/win/winutil.cpp)

set (BEACON_LINUX_SRC ${BEACON_SRC_DIR}/body/cmdexec/cmdexec_linux.cpp
                      ${BEACON_SRC_DIR}/base/inx/inxutil.cpp)

set (PROTOBUF_CPP_SRC ${PROTOBUF_CPP_DIR}/netio.pb.cc
    ${PROTOBUF_CPP_DIR}/taskdata.pb.cc)

set (YASIO_DIR ${BEACON_SRC_DIR}/yasio/)
set (COMMON_INC_DIR ${BEACON_SRC_DIR}/common/)

include_directories(${VcpkgRoot}/include)
include_directories ("${YASIO_DIR}")
include_directories ("${COMMON_INC_DIR}")
include_directories(${BEACON_SRC_DIR}/body)
include_directories(${PROTOBUF_CPP_DIR})

if(MSVC)     
    # Use the static C library for all build types
    foreach(var 
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
      )
      if(${var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
      endif()
    endforeach()    
endif(MSVC)

find_package(OpenSSL REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

if(LINUX)

    set(CMAKE_CXX_FLAGS_MINSIZEREL  "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -s")
    set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -Os -s")

    add_executable(${target_name} ${BEACON_COMM_CORE} ${BEACON_LINUX_SRC} ${PROTOBUF_CPP_SRC})
    target_link_libraries(${target_name}  PRIVATE OpenSSL::Crypto PRIVATE protobuf::libprotobuf-lite PRIVATE dl PRIVATE pthread)
else(LINUX)

    add_executable(${target_name} ${BEACON_COMM_CORE} ${BEACON_WIN_SRC} ${PROTOBUF_CPP_SRC})
    target_link_libraries(${target_name} PRIVATE Crypt32.lib PRIVATE OpenSSL::Crypto PRIVATE protobuf::libprotobuf-lite)
endif(LINUX)