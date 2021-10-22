FROM ubuntu:18.04 AS base_image

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
      vim openssh-client openssh-server \
      ninja-build

RUN apt-get install -y python3 python3-pip python3-wheel && \
    pip3 install pip --upgrade && \
    update-alternatives --install /usr/bin/python python /usr/bin/python3.6 1

# --- Compiler setup
ENV WORKSPACE="/ws"
COPY sources/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz /tmp
RUN cd /tmp && \
    mkdir -p $WORKSPACE && \
    tar -xf gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz -C $WORKSPACE

ENV COMPILER_HOME="$WORKSPACE/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu"
ENV PATH="$COMPILER_HOME/bin:$PATH"
WORKDIR ${WORKSPACE}

# -- Kernel
COPY sources/linux-5.10.72.tar.xz /tmp
COPY configs/aarch64-softmmu_kernel_defconfig /tmp
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-none-linux-gnu-
RUN cd /tmp && \
    tar -xf linux-5.10.72.tar.xz -C $WORKSPACE && \
    cp aarch64-softmmu_kernel_defconfig $WORKSPACE/linux-5.10.72/arch/arm64/configs/ && \
    cd $WORKSPACE/linux-5.10.72 && \
    make aarch64-softmmu_kernel_defconfig && \
    make -j4

# -- Qemu
# NOTE: QEMU archive downloaded from github.com
# does not work. https://lists.gnu.org/archive/html/qemu-devel/2019-05/msg01676.html
FROM base_image AS qemu_image
RUN cd ${WORKSPACE} && \
    mkdir -p qemu_src && \
    cd qemu_src && \
    git clone -b v6.1.0 https://git.qemu.org/git/qemu.git . && \
    ./configure --target-list=aarch64-softmmu && \
    make -j4

# -- buildroot
# NOTE: Buildroot is doing compilation of kernel too however
# we are not using that
FROM base_image AS buildroot_image
COPY sources/linux-5.10.72.tar.xz /tmp
COPY configs/aarch64-softmmu_kernel_defconfig /tmp
COPY sources/2021.08.1.tar.gz /tmp
COPY configs/aarch64-softmmu_buildroot_defconfig /tmp
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-none-linux-gnu-
RUN cd /tmp && \
    tar -xf 2021.08.1.tar.gz -C $WORKSPACE  && \
    mkdir -p $WORKSPACE/buildroot-2021.08.1/board/aarch64-softmmu && \
    cp linux-5.10.72.tar.xz                $WORKSPACE/buildroot-2021.08.1/board/aarch64-softmmu && \
    cp aarch64-softmmu_kernel_defconfig    $WORKSPACE/buildroot-2021.08.1/board/aarch64-softmmu && \
    cp aarch64-softmmu_buildroot_defconfig $WORKSPACE/buildroot-2021.08.1/configs && \
    cd $WORKSPACE/buildroot-2021.08.1 && \
    make aarch64-softmmu_buildroot_defconfig && \
    make -j4

# -- Final working image
FROM base_image AS qemu_workspace
ENV WORKSPACE="/ws"
RUN mkdir -p $WORKSPACE/qemu && \ 
    cd $WORKSPACE/qemu
COPY --from=buildroot_image $WORKSPACE/buildroot-2021.08.1/board/aarch64-softmmu/aarch64-softmmu_kernel_defconfig $WORKSPACE/qemu
COPY --from=buildroot_image $WORKSPACE/buildroot-2021.08.1/configs/aarch64-softmmu_buildroot_defconfig $WORKSPACE/qemu
COPY --from=buildroot_image $WORKSPACE/buildroot-2021.08.1/output/images/rootfs.ext3 $WORKSPACE/qemu
COPY --from=qemu_image $WORKSPACE/qemu_src/build/aarch64-softmmu/qemu-system-aarch64 $WORKSPACE/qemu
COPY --from=qemu_image $WORKSPACE/qemu_src/pc-bios/efi-virtio.rom $WORKSPACE/qemu
COPY configs/run_qemu.sh $WORKSPACE/qemu

# -- Clean
RUN apt-get clean && \
    rm -rf /tmp/* /var/tmp/*

CMD ["/bin/bash"]
