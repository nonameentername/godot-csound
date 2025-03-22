#!/bin/bash

$(eval /osxcross/tools/osxcross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

for ARCH in x86_64 arm64; do
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
done

$dir/scripts/lipo-dir.py  \
    $dir/modules/csound/build/osxcross-arm64/release \
    $dir/modules/csound/build/osxcross-x86_64/release \
    $dir/modules/csound/build/osxcross/release

prefix=$dir/addons/csound/bin/macos/release
prefix_x64=$src_dir/bin/osxcross-x86_64/release
prefix_arm64=$src_dir/bin/osxcross-arm64/release

$dir/scripts/lipo-dir.py $prefix_arm64 $prefix_x64 $prefix

zsign -a $dir/addons/csound/bin/macos/release/Library/Frameworks/CsoundLib64.framework/Versions/Current/CsoundLib64

export OSXCROSS_ROOT=$OSXCROSS_BASE_DIR

cd $dir
scons platform=macos target=template_release osxcross_sdk=$OSXCROSS_TARGET
