FROM debian
MAINTAINER Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>

ENV GT_VER 0.19.8
ENV ROOT_HOME /root

RUN apt-get -y update && apt-get install -y git autoconf automake libtool make g++ texinfo curl

WORKDIR ${ROOT_HOME}
RUN git clone https://github.com/emcrisostomo/fswatch.git
RUN curl -O http://ftp.gnu.org/pub/gnu/gettext/gettext-${GT_VER}.tar.gz
RUN tar -zxf gettext-${GT_VER}.tar.gz && rm gettext-${GT_VER}.tar.gz
RUN cd gettext-${GT_VER} && ./configure && make && make install && ldconfig
RUN rm -r gettext-${GT_VER}

CMD ["/bin/bash"]