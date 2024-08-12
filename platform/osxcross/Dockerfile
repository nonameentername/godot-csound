FROM ubuntu:22.04

ADD dependencies.sh /

RUN /dependencies.sh

RUN ln -s /osxcross/target/bin/x86_64-apple-darwin23-install_name_tool /osxcross/target/bin/install_name_tool
RUN ln -s /osxcross/target/bin/x86_64-apple-darwin23-otool /osxcross/target/bin/otool
