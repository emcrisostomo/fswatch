FROM alpine
MAINTAINER Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>

ENV ROOT_HOME /root
ENV FSWATCH_BRANCH master

RUN apk add --no-cache git autoconf automake libtool gettext gettext-dev make g++ texinfo curl

WORKDIR ${ROOT_HOME}
RUN git clone https://github.com/emcrisostomo/fswatch.git

WORKDIR ${ROOT_HOME}/fswatch
RUN git checkout ${FSWATCH_BRANCH}
RUN ./autogen.sh && ./configure && make -j

CMD ["/bin/bash"]