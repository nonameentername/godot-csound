#!/bin/bash

$(eval /osxcross/tools/osxcross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

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
