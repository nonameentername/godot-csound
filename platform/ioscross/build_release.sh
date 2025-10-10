#!/bin/bash

dir=$(realpath .)

ARCH=arm64 $dir/platform/ioscross/build_deps_release.sh
ARCH=arm64_sim $dir/platform/ioscross/build_deps_release.sh
ARCH=x86_64_sim $dir/platform/ioscross/build_deps_release.sh
$dir/platform/ioscross/build_lib_release.sh
