#!/bin/bash

$(eval $BASE_DIR/ioscross/arm64/ioscross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound
arch=arm64

build_dir=$src_dir/build/ioscross/debug
prefix=$dir/addons/csound/bin/ios/debug

mkdir -p $build_dir
cd $build_dir

cmake -DCUSTOM_CMAKE=$src_dir/platform/ioscross/custom-ios.cmake \
    -DIOS=1 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DUSE_VCPKG=1 \
    -DOSXCROSS_SDK=${OSXCROSS_SDK} \
    -DOSXCROSS_TARGET=${OSXCROSS_TARGET} \
    -Darch=${arch} \
    -DCMAKE_OSX_ARCHITECTURES=${arch} \
    $src_dir

make
make install

arm-apple-${OSXCROSS_TARGET}-install_name_tool -id @rpath/CsoundLib.framework/CsoundLib $prefix/Library/Frameworks/CsoundLib.framework/CsoundLib

for ARCH in x86_64 arm64; do

    $(eval $BASE_DIR/ioscross/${ARCH}_sim/ioscross_conf.sh)
    build_dir=$src_dir/build/ioscross-$ARCH/debug
    prefix=$src_dir/bin/ioscross-$ARCH/debug

    mkdir -p $build_dir
    cd $build_dir

    cmake -DCUSTOM_CMAKE=$src_dir/platform/ioscross/custom-ios.cmake \
        -DIOS=1 \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_VERBOSE_MAKEFILE=1 \
        -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DUSE_VCPKG=1 \
        -DOSXCROSS_SDK=${OSXCROSS_SDK} \
        -DOSXCROSS_TARGET=${OSXCROSS_TARGET} \
        -Darch=${ARCH} \
        -DCMAKE_OSX_ARCHITECTURES=${ARCH} \
        $src_dir

    make
    make install

    if [[ "$ARCH" == "arm64" ]]; then
        arm-apple-${OSXCROSS_TARGET}-install_name_tool -id @rpath/CsoundLib.framework/CsoundLib $prefix/Library/Frameworks/CsoundLib.framework/CsoundLib
    else
        x86_64-apple-${OSXCROSS_TARGET}-install_name_tool -id @rpath/CsoundLib.framework/CsoundLib $prefix/Library/Frameworks/CsoundLib.framework/CsoundLib
    fi
done

prefix=$dir/addons/csound/bin/ios/sim_debug
prefix_x64=$src_dir/bin/ioscross-x86_64/debug
prefix_arm64=$src_dir/bin/ioscross-arm64/debug

mkdir -p $prefix
cp -r $prefix_arm64/* $prefix

lipo -create \
    $prefix_x64/Library/Frameworks/CsoundLib.framework/CsoundLib \
    $prefix_arm64/Library/Frameworks/CsoundLib.framework/CsoundLib \
    -output $prefix/Library/Frameworks/CsoundLib.framework/CsoundLib

lipo -create \
    $prefix_x64/lib/libCsoundLib.a \
    $prefix_arm64/lib/libCsoundLib.a \
    -output $prefix/lib/libCsoundLib.a

for program in $(cd $prefix/bin && ls */cs*); do
    lipo -create \
        $prefix_x64/bin/$program \
        $prefix_arm64/bin/$program \
        -output $prefix/bin/$program
done

$(eval $BASE_DIR/ioscross/arm64/ioscross_conf.sh)

cd $dir
scons platform=ios arch=arm64 target=template_debug dev_build=yes debug_symbols=yes IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="arm-apple-$OSXCROSS_TARGET-"

$(eval $BASE_DIR/ioscross/arm64_sim/ioscross_conf.sh)
scons platform=ios arch=arm64 ios_simulator=yes target=template_debug dev_build=yes debug_symbols=yes IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="arm-apple-$OSXCROSS_TARGET-"

$(eval $BASE_DIR/ioscross/x86_64_sim/ioscross_conf.sh)
scons platform=ios arch=x86_64 ios_simulator=yes target=template_debug dev_build=yes debug_symbols=yes IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="x86_64-apple-$OSXCROSS_TARGET-"

lipo -create \
    addons/csound/bin/ios/libcsoundgodot.ios.template_debug.dev.arm64.simulator.dylib \
    addons/csound/bin/ios/libcsoundgodot.ios.template_debug.dev.x86_64.simulator.dylib \
    -output addons/csound/bin/ios/libcsoundgodot.ios.template_debug.dev.universal.simulator.dylib
