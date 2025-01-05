FROM ubuntu:22.04 AS base-image

ENV OSX_SDK="MacOSX14.0.sdk"
ENV OSX_SDK_URL="https://github.com/joseluisq/macosx-sdks/releases/download/14.0/${OSX_SDK}.tar.xz"

RUN apt update -y
RUN apt install -y autoconf libtool \
    bash-completion \
    build-essential \
    git \
    vim \
    cmake \
    curl \
    zip \
    flex bison \
    clang \
    make \
    libssl-dev \
    liblzma-dev \
    libxml2-dev \
    libz-dev \
    libbz2-dev \
    cpio \
    pkg-config \
    ninja-build \
    python3 \
    python3-pip

RUN pip3 install scons

RUN git clone https://github.com/tpoechtrager/osxcross.git

RUN /osxcross/tools/get_dependencies.sh

RUN curl -sSL "$OSX_SDK_URL" -o "/osxcross/tarballs/$OSX_SDK.tar.xz"

RUN UNATTENDED=1 /osxcross/build_clang.sh

RUN UNATTENDED=1 /osxcross/build.sh

RUN ln -s /osxcross/target/bin/x86_64-apple-darwin23-install_name_tool /osxcross/target/bin/install_name_tool
RUN ln -s /osxcross/target/bin/x86_64-apple-darwin23-otool /osxcross/target/bin/otool

RUN rm -rf /osxcross/tarballs/$OSX_SDK.tar.xz

RUN git clone https://github.com/zhlynn/zsign.git

WORKDIR /zsign

RUN mkdir build

WORKDIR /zsign/build

RUN cmake ..

RUN make

RUN cp /zsign/build/zsign /usr/local/bin

RUN rm -r /zsign

FROM ubuntu:22.04
COPY --from=base-image / /
