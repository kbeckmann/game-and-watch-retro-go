FROM ubuntu

WORKDIR /opt

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install -y \
        make wget curl sudo vim git unzip lz4 \
        python3 libncurses5 \
        && \
    wget -O toolchain.tar.bz2 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2?revision=ca0cbf9c-9de2-491c-ac48-898b5bbc0443&la=en&hash=68760A8AE66026BCF99F05AC017A6A50C6FD832A' && \
    tar xf toolchain.tar.bz2 && \
    rm -f toolchain.tar.bz2 && \
    useradd -m docker && echo "docker:docker" | chpasswd && \
    chown docker:docker /opt && \
    echo "docker ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

ENV GCC_PATH=/opt/gcc-arm-none-eabi-10-2020-q4-major/bin

USER docker

# Install openocd nightly
RUN wget https://nightly.link/kbeckmann/ubuntu-openocd-git-builder/workflows/docker/master/openocd-git.deb.zip && \
    unzip openocd-git.deb.zip && \
    sudo apt -y install ./openocd-git_*_amd64.deb
ENV OPENOCD="/opt/openocd-git/bin/openocd"

RUN mkdir /opt/workdir
RUN sudo chown -R docker:docker /opt/workdir

WORKDIR /opt/workdir

CMD /bin/bash
