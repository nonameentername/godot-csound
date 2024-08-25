#!/bin/bash

dir=$(realpath .)
src_dir=$dir/modules/csound

for ARCH in arm arm64 x86 x64; do
    build_dir=$src_dir/build/android-$ARCH/debug
    prefix=$dir/addons/csound/bin/android-$ARCH/debug

    mkdir -p $build_dir
    cd $build_dir

    cmake -DCUSTOM_CMAKE=$src_dir/platform/android/custom-android.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_VERBOSE_MAKEFILE=1 \
        -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
        -DCMAKE_SYSTEM_NAME=Android \
        -DANDROID=1 \
        -DUSE_VCPKG=1 \
        -DCMAKE_TARGET_ARCHITECTURE=${ARCH} \
        $src_dir

    make
    make install
done

cd $dir

scons platform=android arch=arm32 target=template_debug dev_build=yes debug_symbols=yes android_api_level=23
scons platform=android arch=arm64 target=template_debug dev_build=yes debug_symbols=yes android_api_level=23
scons platform=android arch=x86_32 target=template_debug dev_build=yes debug_symbols=yes android_api_level=23
scons platform=android arch=x86_64 target=template_debug dev_build=yes debug_symbols=yes android_api_level=23
