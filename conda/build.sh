#!/usr/bin/env bash

set -ex

if [[ "$target_platform" == "osx-64" ]]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DCMAKE_MACOSX_RPATH=ON"
fi

mkdir build
cd build

cmake \
    -DRIPC_BUILD_TESTING=OFF \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    ${CMAKE_ARGS} \
    ..

cmake --build . --config RelWithDebInfo -- -j$CPU_COUNT
cmake --build . --config RelWithDebInfo --target install
