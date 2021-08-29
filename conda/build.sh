#!/usr/bin/env bash

set -ex

mkdir build
cd build

cmake \
    -DRIPC_BUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    ..

cmake --build . --config RelWithDebInfo -- -j$CPU_COUNT
cmake --build . --config RelWithDebInfo --target install
