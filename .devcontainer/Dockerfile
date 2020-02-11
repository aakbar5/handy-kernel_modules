FROM ubuntu:18.04

LABEL maintainer="aakbar5"

ENV LANG C.UTF-8
ENV PYTHONUNBUFFERED 1

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      software-properties-common python3-software-properties \
      apt-utils curl wget pkg-config unzip \
      sudo lsb-release pciutils cpio \
      build-essential sudo git \
      file unzip xz-utils rsync bc flex bison pkg-config \
      libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev \
      chrpath gawk texinfo libsdl1.2-dev \
      openssl libssl-dev libncurses-dev \
      vim openssh-client openssh-server

RUN apt-get install -y python3 python3-pip python3-wheel && \
    pip3 install pip --upgrade && \
    update-alternatives --install /usr/bin/python python /usr/bin/python3.6 1

# Use already downloaded sources
# COPY sources/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz /tmp
# COPY sources/linux-5.5.tar.xz /tmp
# COPY sources/2019.08.3.tar.gz /tmp
# RUN cd /tmp && \
#     mkdir -p /ws && \
#     \
#     tar -xf gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz -C /ws/ && \
#     \
#     tar -xf linux-5.5.tar.xz -C /ws/ && \
#     \
#     tar -xf 2019.08.3.tar.gz -C /ws/

# Download and extract sources
RUN cd /tmp && \
    mkdir -p /ws && \
    \
    wget https://releases.linaro.org/components/toolchain/binaries/latest-7/aarch64-linux-gnu/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz && \
    tar -xf gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz -C /ws/ && \
    \
    wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.5.tar.xz && \
    tar -xf linux-5.5.tar.xz -C /ws/ && \
    \
    wget https://github.com/buildroot/buildroot/archive/2019.08.3.tar.gz && \
    tar -xf 2019.08.3.tar.gz -C /ws/

ENV WORKSPACE="/ws"
ENV BUILDROOT_HOME="$WORKSPACE/buildroot-2019.08.3"
ENV LINUX_HOME="$WORKSPACE/linux-5.5"
ENV QEMU_HOME="$WORKSPACE/qemu-4.2.0"
ENV COMPILER_HOME="$WORKSPACE/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu"
ENV PATH="$COMPILER_HOME/bin:$PATH"
WORKDIR ${WORKSPACE}

# COPY defconfig
COPY configs/aarch64-softmmu_buildroot_defconfig ${BUILDROOT_HOME}/configs/
COPY configs/aarch64-softmmu_kernel_defconfig ${LINUX_HOME}/arch/arm64/configs/
COPY configs/run_qemu.sh ${WORKSPACE}

# Build root
RUN cd ${BUILDROOT_HOME} && \
    make aarch64-softmmu_buildroot_defconfig && \
    make -j4

# Build kernel
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-linux-gnu-
RUN cd ${LINUX_HOME} && \
    make aarch64-softmmu_kernel_defconfig && \
    make -j4

# Build Qemu
# NOTE: QEMU archive downloaded from github.com
# does not work. https://lists.gnu.org/archive/html/qemu-devel/2019-05/msg01676.html
RUN cd / && \
    mkdir -p ${QEMU_HOME} && \
    cd ${QEMU_HOME} && \
    git clone https://git.qemu.org/git/qemu.git . && \
    git checkout v4.2.0 && \
    ./configure --target-list=aarch64-softmmu && \
    make -j4

RUN apt-get clean && \
    rm -rf /tmp/* /var/tmp/*

CMD ["/bin/bash"]
