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
project(teamclient)

set(target_name teamclient)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(-DUNICODE -D_UNICODE)

set(VcpkgRoot ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})

set (CLIENT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/client)

#message(${CLIENT_SRC_DIR})

set(CLIENT_UI ${CLIENT_SRC_DIR}/ui/about.ui
${CLIENT_SRC_DIR}/ui/configure.ui
${CLIENT_SRC_DIR}/ui/downloads.ui
${CLIENT_SRC_DIR}/ui/eventlog.ui
${CLIENT_SRC_DIR}/ui/filebrowser.ui
${CLIENT_SRC_DIR}/ui/khepri.ui
${CLIENT_SRC_DIR}/ui/listeners.ui
${CLIENT_SRC_DIR}/ui/process.ui
${CLIENT_SRC_DIR}/ui/shell.ui
${CLIENT_SRC_DIR}/ui/login.ui
${CLIENT_SRC_DIR}/res/client.qrc
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CLIENT_SRC_DIR}/ui)


set (CLIENT_SRC ${CLIENT_SRC_DIR}/main.cpp
${CLIENT_SRC_DIR}/about/about.cpp
${CLIENT_SRC_DIR}/beacon/beacon_msghandler.cpp
${CLIENT_SRC_DIR}/beacon/beacon_req.cpp
${CLIENT_SRC_DIR}/beacon/configure.cpp
${CLIENT_SRC_DIR}/common/qt-wrapper.cpp
${CLIENT_SRC_DIR}/khepri/khepri.cpp
${CLIENT_SRC_DIR}/login/login.cpp
${CLIENT_SRC_DIR}/mq/msg.cpp
${CLIENT_SRC_DIR}/mq/queue.cpp
${CLIENT_SRC_DIR}/rpclient/rpclient.cpp
${CLIENT_SRC_DIR}/tabwidget/downloads.cpp
${CLIENT_SRC_DIR}/tabwidget/eventlog.cpp
${CLIENT_SRC_DIR}/tabwidget/filebrowser.cpp
${CLIENT_SRC_DIR}/tabwidget/listeners.cpp
${CLIENT_SRC_DIR}/tabwidget/process.cpp
${CLIENT_SRC_DIR}/tabwidget/shell.cpp
${CLIENT_SRC_DIR}/tabwidget/tabwidget.cpp
)

set (PROTOBUF_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto_autogen_cpp)

set (PROTOBUF_CPP_SRC ${PROTOBUF_CPP_DIR}/client.pb.cc
${PROTOBUF_CPP_DIR}/taskdata.pb.cc
${PROTOBUF_CPP_DIR}/teamrpc.grpc.pb.cc
${PROTOBUF_CPP_DIR}/teamrpc.pb.cc
)

include_directories(${VcpkgRoot}/include)
include_directories(${PROTOBUF_CPP_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../beacon/common)

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
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif(MSVC)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(${target_name} ${CLIENT_SRC}
${CLIENT_UI}
${PROTOBUF_CPP_SRC}
)

target_link_libraries(${target_name}  Qt5::Core Qt5::Gui Qt5::Widgets gRPC::grpc gRPC::grpc++)

