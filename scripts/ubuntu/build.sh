#!/bin/bash

dir=$(realpath bin)
mkdir -p modules/csound/build
cd modules/csound/build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$dir/csound -DBUILD_JAVA_INTERFACE=OFF ..
make
make install
