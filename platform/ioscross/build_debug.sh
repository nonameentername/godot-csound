#!/bin/bash

dir=$(realpath .)

ARCH=arm64 $dir/platform/ioscross/build_deps_debug.sh
ARCH=arm64_sim $dir/platform/ioscross/build_deps_debug.sh
ARCH=x86_64_sim $dir/platform/ioscross/build_deps_debug.sh
$dir/platform/ioscross/build_lib_debug.sh
