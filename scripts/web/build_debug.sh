#!/bin/bash

. /emsdk/emsdk_env.sh

dir=$(realpath .)
src_dir=$dir/modules/csound
build_dir=$src_dir/build/web/debug
prefix=$dir/bin/web/debug

mkdir -p $build_dir
cd $build_dir

emcmake cmake -DCMAKE_TOOLCHAIN_FILE=$src_dir/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
    -DCUSTOM_CMAKE=$src_dir/platform/wasm/Custom-wasm.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_MODULE_PATH=${EMSDK}/upstream/emscripten/cmake/Modules \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DDIRENT_INCLUDE_DIR=${EMSDK}/upstream/emscripten/cache/sysroot \
    -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
    -DCMAKE_SYSTEM_NAME=Emscripten \
    -DCMAKE_CXX_COMPILER=${EMSDK}/upstream/emscripten/em++ \
    -DCMAKE_C_COMPILER=${EMSDK}/upstream/emscripten/emcc \
    -DVCPKG_TARGET_TRIPLET=wasm32-emscripten \
    -DUSE_VCPKG=1 \
    -DBUILD_JAVA_INTERFACE=OFF \
    -DINSTALL_PYTHON_INTERFACE=OFF \
    -DBUILD_UTILITIES=OFF \
    -DBUILD_TESTS=OFF \
    $src_dir

make
make install

cd $dir
scons platform=web target=template_debug dev_build=yes debug_symbols=yes
