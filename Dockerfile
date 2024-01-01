FROM ubuntu:22.04
ARG FSWATCH_VERSION="1.17.1"
RUN apt-get -y update && \
    apt-get install -y bash git autoconf automake gettext autopoint libtool make g++ texinfo curl zip

ENV ROOT_HOME /root
WORKDIR ${ROOT_HOME}/fswatch
COPY . .
RUN ./autogen.sh && ./configure && make -j && make install
WORKDIR /package
RUN cp /usr/local/bin/fswatch /package/ && \
    cp -r /usr/local/lib/*fswatch*.so.* /package/ && \
    mkdir /artifacts && \
    tar -czvf /artifacts/fswatch-${FSWATCH_VERSION}.tar.gz . && \
    zip -r /artifacts/fswatch-${FSWATCH_VERSION}.zip .    
CMD ["/bin/bash"]
