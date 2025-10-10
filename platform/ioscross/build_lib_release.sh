#!/bin/bash

$(eval /ioscross/arm64/ioscross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

$dir/scripts/lipo-dir.py  \
    $src_dir/build/ioscross-arm64/release \
    $src_dir/build/ioscross-x86_64/release \
    $src_dir/build/ioscross-universal/release

prefix=$dir/addons/csound/bin/ios/sim_release
prefix_x64=$src_dir/bin/ioscross-x86_64/release
prefix_arm64=$src_dir/bin/ioscross-arm64/release

$dir/scripts/lipo-dir.py $prefix_arm64 $prefix_x64 $prefix

$(eval /ioscross/arm64/ioscross_conf.sh)

cd $dir
scons platform=ios arch=arm64 target=template_release IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="arm-apple-$OSXCROSS_TARGET-"

$(eval /ioscross/arm64_sim/ioscross_conf.sh)
scons platform=ios arch=arm64 ios_simulator=yes target=template_release IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="arm-apple-$OSXCROSS_TARGET-"

$(eval /ioscross/x86_64_sim/ioscross_conf.sh)
scons platform=ios arch=x86_64 ios_simulator=yes target=template_release IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="x86_64-apple-$OSXCROSS_TARGET-"

lipo -create \
    addons/csound/bin/ios/libcsoundgodot.ios.template_release.arm64.simulator.dylib \
    addons/csound/bin/ios/libcsoundgodot.ios.template_release.x86_64.simulator.dylib \
    -output addons/csound/bin/ios/libcsoundgodot.ios.template_release.universal.simulator.dylib
