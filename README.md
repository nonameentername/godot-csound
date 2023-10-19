godot-csound
============

Godot gdextension csound library to allow playing music using csound.  Currently works with Godot v4.1.1 stable release.

How to Install
--------------

The first thing to do is make sure you have all of the required dependencies for
building Csound. In a terminal, do the following:

Edit, as root, `/etc/apt/sources.list` and ensure that each line beginning with `deb` has another line below it that is identical except that `deb` is replaced with `deb-src`. Then run:

    sudo apt update
    sudo apt build-dep csound
    sudo apt install cmake
    sudo apt install git

Install system dependencies for Ubuntu:

    sudo apt install abcmidi

Build
-----

Initialize git submodules:

    git submodule update --init --recursive

Create assets:

    make assets

Compile csound:

    make csound

Compile godot-cpp library:

    make godot-cpp

Compile gdextension library:

    make
