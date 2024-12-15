godot-csound
============

[![builds](https://github.com/nonameentername/godot-csound/actions/workflows/builds.yml/badge.svg)](https://github.com/nonameentername/godot-csound/actions/workflows/builds.yml)
[![docker_builds](https://github.com/nonameentername/godot-csound/actions/workflows/build_images.yml/badge.svg)](https://github.com/nonameentername/godot-csound/actions/workflows/build_images.yml)

Godot gdextension csound library to allow playing music using csound.  Currently works with Godot v4.3 stable release.

Project running in the browser [godot-csound](https://nonameentername.github.io/godot-csound/csoundgodot.html)

Projects using godot-csound
---------------------------

* [godot-csound-example](https://github.com/nonameentername/godot-csound-example) - Simple godot-csound example.
* [godot-synths](https://github.com/nonameentername/godot-synths) - Music synths for Godot.  ([demo](https://nonameentername.github.io/godot-synths/godot-synths.html))

How to Install
--------------

This project uses docker to build the project for different platforms.
The build scripts were developed using Ubuntu (x86_64).


## Instructions

1. Install docker using apt:

```bash
sudo apt install docker.io
```

2. Add user to docker group:

```bash
sudo usermod -a -G docker $USER
```

3. Initialize git submodules:

```bash
git submodule update --init --recursive
```

4. Build project using make:

```bash
#build all platforms
make all

#build for ubuntu
make ubuntu

#build for windows
make mingw

#build for MacOS
make osxcross

#build for iOS
make ioscross

#build for Android
make android

#build for web
make web
```
