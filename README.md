godot-csound
============

Godot gdextension csound library to allow playing music using csound.  Currently works with Godot v4.2.0 stable release.

How to Install
--------------

The first thing to do is make sure you have all of the required dependencies for
building Csound.

Ubuntu
------

In a terminal, run the following:

    ./scripts/ubuntu/dependencies.sh

Windows
-------

In powershell run the following as an Administator:

    ./scripts/windows/system-dependencies.ps1
    ./scripts/windows/dependencies.ps1

Build
-----

Initialize git submodules:

    git submodule update --init --recursive

Compile csound:

    make csound

Compile gdextension library:

    make
