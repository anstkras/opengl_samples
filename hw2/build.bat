@ECHO ON

RMDIR /Q /S build
MKDIR build
PUSHD build

REM conan install .. -s build_type=Debug
REM cmake .. -G "Visual Studio 15 2017 Win64"
REM cmake --build . --config Debug

conan install .. -s build_type=Release
cmake .. -G "Visual Studio 15 2017 Win64"
cmake --build . --config Release
