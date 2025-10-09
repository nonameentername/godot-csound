#!/bin/bash

$(eval /osxcross/tools/osxcross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

$dir/scripts/lipo-dir.py  \
    $dir/modules/csound/build/osxcross-arm64/debug \
    $dir/modules/csound/build/osxcross-x86_64/debug \
    $dir/modules/csound/build/osxcross/debug

prefix=$dir/addons/csound/bin/macos/debug
prefix_x64=$src_dir/bin/osxcross-x86_64/debug
prefix_arm64=$src_dir/bin/osxcross-arm64/debug

$dir/scripts/lipo-dir.py $prefix_arm64 $prefix_x64 $prefix

zsign -a $dir/addons/csound/bin/macos/debug/Library/Frameworks/CsoundLib64.framework/Versions/Current/CsoundLib64

export OSXCROSS_ROOT=$OSXCROSS_BASE_DIR

cd $dir
scons platform=macos target=template_debug dev_build=yes debug_symbols=yes osxcross_sdk=$OSXCROSS_TARGET
