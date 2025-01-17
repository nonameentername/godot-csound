#!/bin/bash

echo TAG_VERSION=$TAG_VERSION
echo BUILD_SHA=$BUILD_SHA

dir=$(realpath .)
src_dir=$dir/modules/csound
build_dir=$src_dir/build/linux/debug

mkdir -p $build_dir
cd $build_dir

cmake -DCUSTOM_CMAKE=$src_dir/platform/linux/custom.cmake \
    -DCMAKE_INSTALL_PREFIX:PATH=$dir/addons/csound/bin/linux/debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DUSE_VCPKG=1 \
    -DBUILD_JAVA_INTERFACE=OFF \
    -DINSTALL_PYTHON_INTERFACE=OFF \
    -DBUILD_PLUGINS=OFF \
    $src_dir

make
make install

cd $dir
scons platform=linux target=template_debug dev_build=yes debug_symbols=yes
