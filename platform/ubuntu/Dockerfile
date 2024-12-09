FROM ubuntu:22.04  AS base-image

RUN sed -i 's/^# deb-src/deb-src/g' /etc/apt/sources.list
RUN apt update -y
RUN apt build-dep -y csound
RUN apt install -y build-essential
RUN apt install -y cmake
RUN apt install -y git
RUN apt install -y python3
RUN apt install -y python3-pip
RUN pip3 install scons

FROM ubuntu:22.04
COPY --from=base-image / /
