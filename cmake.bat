@echo off

Rem Run this only to build the release.

if exist build\ (
  rmdir /s /q "build"
)

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release