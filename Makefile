UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM=linux
else ifeq ($(UNAME), Darwin)
PLATFORM=osx
else
PLATFORM=windows
endif

release-build:
	scons platform=$(PLATFORM) target=template_release

dev-build:
	scons platform=$(PLATFORM) target=template_debug dev_build=yes debug_symbols=yes

godot-cpp:
	(cd godot-cpp && scons platform=$(PLATFORM) bits=64 generate_bindings=yes)

format:
	clang-format -i src/*.cpp src/*.h
	gdformat $(shell find -name '*.gd' ! -path './godot-cpp/*')

clean:
	rm -f src/*.os

compiledb: clean
	scons -c platform=$(PLATFORM) target=template_debug dev_build=yes debug_symbols=yes | tee build-log.txt
	scons platform=$(PLATFORM) target=template_debug dev_build=yes debug_symbols=yes | tee build-log.txt
	compiledb --parse build-log.txt

UNAME := $(shell uname)
ifeq ($(UNAME), Windows)
    UID=1000
    GID=1000
else
    UID=`id -u`
    GID=`id -g`
endif

SHELL_COMMAND = bash

docker-ubuntu:
	docker build -t godot-csound-ubuntu ./platform/ubuntu

shell-ubuntu: docker-ubuntu
	docker run -it --rm -v ${CURDIR}:${CURDIR} --user ${UID}:${GID} -w ${CURDIR} godot-csound-ubuntu ${SHELL_COMMAND}

ubuntu:
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_release.sh'
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_debug.sh'

ubuntu-debug:
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_debug.sh'

docker-web:
	docker build -t godot-csound-web ./platform/web

shell-web: docker-web
	docker run -it --rm -v ${CURDIR}:${CURDIR} --user ${UID}:${GID} -w ${CURDIR} godot-csound-web ${SHELL_COMMAND}

web:
	$(MAKE) shell-web SHELL_COMMAND='./platform/web/build_release.sh'
	$(MAKE) shell-web SHELL_COMMAND='./platform/web/build_debug.sh'

docker-mingw:
	docker build -t godot-csound-mingw ./platform/mingw

shell-mingw: docker-mingw
	docker run -it --rm -v ${CURDIR}:${CURDIR} --user ${UID}:${GID} -w ${CURDIR} godot-csound-mingw ${SHELL_COMMAND}

mingw:
	$(MAKE) shell-mingw SHELL_COMMAND='./platform/mingw/build_release.sh'
	$(MAKE) shell-mingw SHELL_COMMAND='./platform/mingw/build_debug.sh'

docker-osxcross:
	docker build -t godot-csound-osxcross ./platform/osxcross

shell-osxcross: docker-osxcross
	docker run -it --rm -v ${CURDIR}:${CURDIR} --user ${UID}:${GID} -w ${CURDIR} godot-csound-osxcross ${SHELL_COMMAND}

osxcross:
	$(MAKE) shell-osxcross SHELL_COMMAND='./platform/osxcross/build_release.sh'
	$(MAKE) shell-osxcross SHELL_COMMAND='./platform/osxcross/build_debug.sh'

docker-ioscross:
	docker build -t godot-csound-ioscross ./platform/ioscross

shell-ioscross: docker-ioscross
	docker run -it --rm -v ${CURDIR}:${CURDIR} --user ${UID}:${GID} -w ${CURDIR} godot-csound-ioscross ${SHELL_COMMAND}

ioscross:
	$(MAKE) shell-ioscross SHELL_COMMAND='./platform/ioscross/build_release.sh'
	$(MAKE) shell-ioscross SHELL_COMMAND='./platform/ioscross/build_debug.sh'

docker-windows:
	docker build -t godot-csound-windows ./platform/windows

shell-windows: docker-windows
	docker run -it --rm -v ${CURDIR}:${CURDIR} -w ${CURDIR} godot-csound-windows powershell

all: ubuntu mingw osxcross ioscross #web 

cgdb:
	cgdb --args $GODOT4 --editor
