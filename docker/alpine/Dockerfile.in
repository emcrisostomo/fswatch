FROM alpine
MAINTAINER Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>

RUN apk add --no-cache file git autoconf automake libtool gettext gettext-dev make g++ texinfo curl

ENV ROOT_HOME /root
ENV FSWATCH_BRANCH @ax_git_current_branch@

WORKDIR ${ROOT_HOME}
RUN git clone https://github.com/emcrisostomo/fswatch.git

WORKDIR ${ROOT_HOME}/fswatch
RUN git checkout ${FSWATCH_BRANCH}
RUN ./autogen.sh && ./configure && make -j

CMD ["/bin/bash"]
