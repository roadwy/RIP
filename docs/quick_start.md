#### Install compiling tool chain

vcpkg、gcc、g++、cmake、vs2015、go 1.14、goland、clion、protobuf protoc、grpc cpp/golang plugin

#### Install dependent open source code

**beacon**: cryptopp、protobuf

**teamclient**：grpc、qt5-base

**teamserver**: go mod tidy、go get all

The C/C++ language use vcpkg  to install dependent code, Please refer https://github.com/Microsoft/vcpkg#quick-start.

#### Clone and Generate the ProtoBuffer code
- git clone --recursive https://github.com/geemion/Khepri
- proto: use *.proto file to generate protobuf and grpc code

```
//generate golang protobuf auto code
protoc -I=[Khepri Proto Dir] --go_out=[Khepri Src Dir] [Khepri Proto Dir]\client.proto
protoc -I=[Khepri Proto Dir] --go_out=[Khepri Src Dir] [Khepri proto dir]\netio.proto
protoc -I=[Khepri Proto Dir] --go_out=[Khepri Src Dir] [Khepri proto dir]\taskdata.proto
protoc -I=[Khepri Proto Dir] --go_out=plugins=grpc:[Khepri Src Dir] -I=[Khepri Proto Dir]\teamrpc.proto

//generate cpp protobuf auto code
protoc -I=[Khepri Proto Dir] --cpp_out=[Khepri Proto Dir]\..\src\proto_autogen_cpp [Khepri Proto Dir]\client.proto
protoc -I=[Khepri Proto Dir] --cpp_out=[Khepri Proto Dir]\..\src\proto_autogen_cpp [Khepri Proto Dir]\netio.proto
protoc -I=[Khepri Proto Dir] --cpp_out=[Khepri Proto Dir]\..\src\proto_autogen_cpp [Khepri Proto Dir]\taskdata.proto
protoc -I=[Khepri Proto Dir] --cpp_out=[Khepri Proto Dir]\..\src\proto_autogen_cpp [Khepri Proto Dir]\teamrpc.proto
protoc -I=[Khepri Proto Dir] --grpc_out="[Khepri Proto Dir]\..\src\proto_autogen_cpp" --plugin=protoc-gen-grpc="[Khepri Proto Dir]\grpc_cpp_plugin.exe" [Khepri Proto Dir]\teamrpc.proto
```
**NOTE: The protoc executable file must copy from protobuf dir([vcpkg root]\installed\x64-windows-static\tools\protobuf\protoc.exe). otherwise there will get a compile error, like https://github.com/geemion/Khepri/issues/2.**

#### Compile
- beacon:
windows: gen-vc-project.bat
```
cmake ^
    -S . ^
    -B buildvc ^
    -G "Visual Studio 14 2015" ^
    -DCMAKE_SYSTEM_VERSION=8.1 ^
    -DCMAKE_TOOLCHAIN_FILE="[vcpkg root]/scripts/buildsystems/vcpkg.cmake" ^       #edit it
    -DVCPKG_TARGET_TRIPLET="x86-windows-static"

cd buildvc
cmake --build . --target beacon --config MinSizeRel
pause
```

linux:gen-mk-project.sh
```
#!/usr/bin/env bash
rm -rf buildmk/
echo "mkdir buildmk"
mkdir buildmk
cmake -S .\
 -B buildmk\
 -DCMAKE_BUILD_TYPE="Debug"\
 -DCMAKE_INSTALL_PREFIX="/usr/local"\
 -DCMAKE_TOOLCHAIN_FILE="[vcpkg root]/scripts/buildsystems/vcpkg.cmake"\       #edit it
 -DVCPKG_TARGET_TRIPLET="x64-linux"\
 -DCMAKE_MAKE_PROGRAM="make"\
 -DCMAKE_C_COMPILER="gcc"\
 -DCMAKE_CXX_COMPILER="g++"\
 --debug-output

cd buildmk
make
```
- teamclient:
windows: gen-vc-project.bat
```
cmake ^
    -S . ^
    -B buildvc64 ^
    -G "Visual Studio 14 2015 Win64" ^
    -DCMAKE_SYSTEM_VERSION=8.1 ^
    -DCMAKE_TOOLCHAIN_FILE="[vcpkg root]/scripts/buildsystems/vcpkg.cmake" ^       #edit it
    -DVCPKG_TARGET_TRIPLET="x64-windows-static"

cd buildvc64
cmake --build . --target teamclient --config MinSizeRel
pause
```

- CLion:
```
Open the Toolchains settings (File > Settings on Windows and Linux, CLion > Preferences), and go to the CMake settings (Build, Execution, Deployment > CMake). Finally, in CMake options, add the following line:

-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
```

- teamserver:
```
go build src\teamserver\cmd\teamserver\teamserver.go
```

#### Run
teamserver
```
Usage of teamserver.exe:
  --privatekey string
        beacon rsa private key file, default:privatekey.pem (default "privatekey.pem")
  --pubkey string
        beacon rsa public key file, default:publickey.pem (default "publickey.pem")
  -d string
        default sqlite3 db file, default:khepri.db (default "khepri.db")
  -h    help usage
  -l string
        teamserver listen at addr, default:0.0.0:50051 (default "0.0.0.0:50051")
  -p string
        teamclient connect password
```