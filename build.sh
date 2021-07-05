#!/bin/sh

echo "clean build dir"
rm -rf build
rm -rf release

echo "generate makefile"
mkdir build && cd build ||exit


#build debug
#cmake  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../debug -G "CodeBlocks - Unix Makefiles" ../
#build release
cmake  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../release -G "CodeBlocks - Unix Makefiles" ../

echo "build starting ..."
echo "build thread:$(nproc)"
make all -j"$(nproc)"
echo "build finished!"

echo "release bin&lib"
rm -rf ../release/*
make install

cd - || exit

