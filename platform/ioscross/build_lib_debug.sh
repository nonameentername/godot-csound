#!/bin/bash

$(eval /ioscross/arm64/ioscross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

$dir/scripts/lipo-dir.py  \
    $src_dir/build/ioscross-arm64/debug \
    $src_dir/build/ioscross-x86_64/debug \
    $src_dir/build/ioscross-universal/debug

prefix=$dir/addons/csound/bin/ios/sim_debug
prefix_x64=$src_dir/bin/ioscross-x86_64/debug
prefix_arm64=$src_dir/bin/ioscross-arm64/debug

$dir/scripts/lipo-dir.py $prefix_arm64 $prefix_x64 $prefix

$(eval /ioscross/arm64/ioscross_conf.sh)

cd $dir
scons platform=ios arch=arm64 target=template_debug dev_build=yes debug_symbols=yes IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="arm-apple-$OSXCROSS_TARGET-"

$(eval /ioscross/arm64_sim/ioscross_conf.sh)
scons platform=ios arch=arm64 ios_simulator=yes target=template_debug dev_build=yes debug_symbols=yes IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="arm-apple-$OSXCROSS_TARGET-"

$(eval /ioscross/x86_64_sim/ioscross_conf.sh)
scons platform=ios arch=x86_64 ios_simulator=yes target=template_debug dev_build=yes debug_symbols=yes IOS_SDK_PATH=$OSXCROSS_SDK IOS_TOOLCHAIN_PATH=$OSXCROSS_TARGET_DIR ios_triple="x86_64-apple-$OSXCROSS_TARGET-"

lipo -create \
    addons/csound/bin/ios/libcsoundgodot.ios.template_debug.dev.arm64.simulator.dylib \
    addons/csound/bin/ios/libcsoundgodot.ios.template_debug.dev.x86_64.simulator.dylib \
    -output addons/csound/bin/ios/libcsoundgodot.ios.template_debug.dev.universal.simulator.dylib
