#!/usr/bin/env bash
rm -rf buildmk/
echo "mkdir buildmk"
mkdir buildmk
cmake -S .\
 -B buildmk\
 -DCMAKE_BUILD_TYPE="Debug"\
 -DCMAKE_INSTALL_PREFIX="/usr/local"\
 -DCMAKE_TOOLCHAIN_FILE="[vcpkg root]/scripts/buildsystems/vcpkg.cmake"\
 -DVCPKG_TARGET_TRIPLET="x64-linux"\
 -DCMAKE_MAKE_PROGRAM="make"\
 -DCMAKE_C_COMPILER="gcc"\
 -DCMAKE_CXX_COMPILER="g++"\
 --debug-output

cd buildmk
make