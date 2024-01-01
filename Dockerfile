FROM ubuntu:22.04

RUN apt-get -y update && \
    apt-get install -y bash git autoconf automake gettext autopoint libtool make g++ texinfo curl

ENV ROOT_HOME /root
WORKDIR ${ROOT_HOME}/fswatch
COPY . .
RUN ./autogen.sh && ./configure && make -j && make install
CMD ["/bin/bash"]
