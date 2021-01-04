#!/bin/bash
mkdir -p build
pushd build
cmake -G"Ninja Multi-Config" -DCMAKE_CXX_COMPILER=g++-10 -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
popd
