#!/bin/bash

dir=$(realpath .)

ARCH=x86_64 $dir/platform/osxcrossbuild_deps_release.sh
ARCH=arm64 $dir/platform/osxcross/build_deps_release.sh
$dir/platform/osxcross/build_lib_release.sh
