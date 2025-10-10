#!/bin/bash

ARCH_SIM=$ARCH
ARCH=${ARCH/_sim/}

$(eval /ioscross/${ARCH_SIM}/ioscross_conf.sh)

dir=$(realpath .)
src_dir=$dir/modules/csound

if [ ${ARCH_SIM} == ${ARCH} ]; then
    build_dir=$src_dir/build/ioscross/release
    prefix=$dir/addons/csound/bin/ios/release
else
    build_dir=$src_dir/build/ioscross-$ARCH/release
    prefix=$src_dir/bin/ioscross-$ARCH/release
fi

mkdir -p $build_dir
cd $build_dir

cmake -DCUSTOM_CMAKE=$src_dir/platform/ioscross/custom-ios.cmake \
    -DIOS=1 \
    -DCMAKE_BUILD_TYPE=Release \
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
