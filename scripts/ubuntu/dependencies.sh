#!/bin/bash

sed -i 's/^# deb-src/deb-src/g' /etc/apt/sources.list
apt update -y
apt build-dep -y csound
apt install -y build-essential
apt install -y cmake
apt install -y git
apt install -y python3
apt install -y python3-pip
pip3 install scons
