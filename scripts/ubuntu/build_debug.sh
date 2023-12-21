#!/bin/bash

dir=$(realpath .)
src_dir=$dir/modules/csound
build_dir=$src_dir/build/linux/debug

mkdir -p $build_dir
cd $build_dir
cmake -DCMAKE_INSTALL_PREFIX:PATH=$dir/bin/linux/debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_JAVA_INTERFACE=OFF \
    -DINSTALL_PYTHON_INTERFACE=OFF \
    $src_dir

make
make install

cd $dir
scons platform=linux target=template_debug dev_build=yes debug_symbols=yes
