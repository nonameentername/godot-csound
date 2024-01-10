#!/bin/bash

dir=$(realpath .)
src_dir=$dir/modules/csound
build_dir=$src_dir/build/linux/release

mkdir -p $build_dir
cd $build_dir
cmake -DCMAKE_INSTALL_PREFIX:PATH=$dir/bin/linux/release \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_JAVA_INTERFACE=OFF \
    -DINSTALL_PYTHON_INTERFACE=OFF \
    $src_dir

make
make install

cd $dir
scons platform=linux target=template_release
