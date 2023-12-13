UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM=linux
else ifeq ($(UNAME), Darwin)
PLATFORM=osx
else
PLATFORM=windows
endif

all:
	scons platform=$(PLATFORM)

dev-build:
	scons platform=$(PLATFORM) dev_build=yes debug_symbols=yes

godot-cpp:
	(cd godot-cpp && scons platform=$(PLATFORM) bits=64 generate_bindings=yes)

.ONESHELL:
csound:
ifeq ($(PLATFORM), linux)
	./scripts/ubuntu/build.sh
else
	./scripts/windows/build.sh
endif

format:
	clang-format -i src/*.cpp src/*.h
	dotnet format ./CsoundGodot.sln
	gdformat $(shell find -name '*.gd' ! -path './godot-cpp/*')

clean:
	rm src/*.os

compiledb: clean
	scons platform=$(PLATFORM) | tee build-log.txt
	compiledb --parse build-log.txt
