#!/bin/bash

$(eval /osxcross/tools/osxcross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

build_dir=$src_dir/build/osxcross-$ARCH/release
prefix=$src_dir/bin/osxcross-$ARCH/release

mkdir -p $build_dir
cd $build_dir

cmake -DCUSTOM_CMAKE=$src_dir/platform/osxcross/custom-osx.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
    -DCMAKE_SYSTEM_NAME=Darwin \
    -DUSE_VCPKG=1 \
    -DOSXCROSS_TARGET_DIR=${OSXCROSS_TARGET_DIR} \
    -DOSXCROSS_SDK=${OSXCROSS_SDK} \
    -DOSXCROSS_TARGET=${OSXCROSS_TARGET} \
    -DCMAKE_OSX_ARCHITECTURES=${ARCH} \
    -DBUILD_PLUGINS=OFF \
    $src_dir

make
make install
