FROM debian
MAINTAINER Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>

ENV GETTEXT_VER 0.19.8
ENV ROOT_HOME /root
ENV FSWATCH_BRANCH master

RUN apt-get -y update && apt-get install -y git autoconf automake libtool make g++ texinfo curl

WORKDIR ${ROOT_HOME}
RUN git clone https://github.com/emcrisostomo/fswatch.git
RUN curl -O http://ftp.gnu.org/pub/gnu/gettext/gettext-${GETTEXT_VER}.tar.gz
RUN tar -zxf gettext-${GETTEXT_VER}.tar.gz && rm gettext-${GETTEXT_VER}.tar.gz
RUN cd gettext-${GETTEXT_VER} && ./configure && make && make install && ldconfig
RUN rm -r gettext-${GETTEXT_VER}

WORKDIR ${ROOT_HOME}/fswatch
RUN git checkout ${FSWATCH_BRANCH}
RUN ./autogen.sh && ./configure && make -j

CMD ["/bin/bash"]