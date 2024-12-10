FROM ubuntu:22.04 AS base-image

RUN apt update -y
RUN apt install -y mingw-w64-x86-64-dev
RUN apt install -y autoconf libtool
RUN apt install -y bash-completion
RUN apt install -y build-essential
RUN apt install -y gcc-mingw-w64
RUN apt install -y gcc-mingw-w64 g++-mingw-w64
RUN apt install -y git
RUN apt install -y mingw-w64-tools
RUN apt install -y vim
RUN apt install -y cmake
RUN apt install -y curl
RUN apt install -y zip
RUN apt install -y flex bison
RUN apt install -y python3
RUN apt install -y python3-pip
RUN pip3 install scons

RUN update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
RUN update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

# Install PowerShell
RUN apt install -y wget apt-transport-https software-properties-common
RUN . /etc/os-release && wget -q https://packages.microsoft.com/config/ubuntu/$VERSION_ID/packages-microsoft-prod.deb
RUN dpkg -i packages-microsoft-prod.deb
RUN rm packages-microsoft-prod.deb
RUN apt update

RUN apt install -y powershell

FROM ubuntu:22.04
COPY --from=base-image / /
