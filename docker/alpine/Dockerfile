FROM alpine
MAINTAINER Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>

ENV GT_VER 0.19.8
ENV ROOT_HOME /root

RUN apk add --no-cache git autoconf automake libtool gettext gettext-dev make g++ texinfo curl

WORKDIR ${ROOT_HOME}
RUN git clone https://github.com/emcrisostomo/fswatch.git

CMD ["/bin/bash"]