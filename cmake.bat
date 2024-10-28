@echo off

Rem Run this only to build the release.

if exist build\ (
  rmdir /s /q "build"
)

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

cmake --build . --config Release --target clisw-launcher
cmake --build . --config Release --target clisw-installer

cmake --build . --config Release --target package