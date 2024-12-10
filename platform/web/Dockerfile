FROM ubuntu:22.04 AS base-image

RUN apt update -y
RUN apt install -y cmake
RUN apt install -y wget
RUN apt install -y build-essential
RUN apt install -y git
RUN apt install -y llvm
RUN apt install -y vim
RUN apt install -y curl zip unzip tar
RUN apt install -y pkg-config
RUN apt install -y autoconf libtool
RUN apt install -y flex bison
RUN apt install -y python3
RUN apt install -y python3-pip
RUN pip3 install scons

RUN git clone https://github.com/emscripten-core/emsdk.git

WORKDIR emsdk
RUN ./emsdk install 3.1.64
RUN ./emsdk activate 3.1.64

FROM ubuntu:22.04
COPY --from=base-image / /
