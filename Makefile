UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM=linux
endif
ifeq ($(UNAME), Darwin)
PLATFORM=osx
endif

all:
	scons platform=$(PLATFORM)

dev-build:
	scons platform=$(PLATFORM) dev_build=yes debug_symbols=yes

godot-cpp:
	(cd godot-cpp && scons platform=$(PLATFORM) bits=64 generate_bindings=yes)

.ONESHELL:
csound:
	dir=$(realpath bin)
	mkdir -p modules/csound/build
	cd modules/csound/build
	cmake -DCMAKE_INSTALL_PREFIX:PATH=$$dir/csound -DBUILD_JAVA_INTERFACE=OFF ..
	make -j 20
	make install

.PHONY: assets godot-cpp
assets:
	cp /usr/share/sounds/sf2/FluidR3_GM.sf2 assets/example.sf2
	abc2midi assets/example.abc -o assets/example.mid
	abc2midi assets/example2.abc -o assets/example2.mid

format:
	clang-format -i src/*.cpp src/*.h
	dotnet format ./MidiPlayer.sln
	gdformat $(shell find -name '*.gd' ! -path './godot-cpp/*')

clean:
	rm src/*.os

compiledb: clean
	scons platform=$(PLATFORM) | tee build-log.txt
	compiledb --parse build-log.txt
