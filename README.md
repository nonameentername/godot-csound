godot-csound
============

Godot gdextension csound library to allow playing music using csound.  Currently works with Godot v4.2.1 stable release.

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
make all
```
