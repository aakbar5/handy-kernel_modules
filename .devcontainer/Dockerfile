FROM ubuntu:24.04 AS base_image

LABEL maintainer="asadmalik5"

ENV LANG="C.UTF-8"
ENV PYTHONUNBUFFERED="1"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        apt-utils sudo lsb-release vim \
        curl wget pkg-config unzip pciutils cpio \
        chrpath gawk texinfo \
        build-essential git file unzip xz-utils rsync bc flex bison pkg-config ninja-build \
        libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev libsdl1.2-dev libslirp-dev \
        openssl libssl-dev libncurses-dev \
        openssh-client openssh-server \
        python3-setuptools python3-venv

ENV WORKSPACE="/ws"
WORKDIR ${WORKSPACE}

# --- Qemu
# NOTE: QEMU archive downloaded from github.com
# does not work. https://lists.gnu.org/archive/html/qemu-devel/2019-05/msg01676.html
# git clone -b v10.0.2 https://git.qemu.org/git/qemu.git
FROM base_image AS qemu_image
COPY sources/qemu_v10.0.2.tar.xz /tmp
RUN cd /tmp && \
    mkdir -p $WORKSPACE && \    
    tar -xf qemu_v10.0.2.tar.xz -C $WORKSPACE && \
    cd $WORKSPACE/qemu_v10.0.2 && \
    ./configure --target-list=aarch64-softmmu --disable-docs && \
    make -j4

FROM base_image AS buildroot_image

# --- Setup versions
ENV COMPILER_SRC="arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu"
ENV BUILDROOT_SRC="buildroot-2025.05"
ENV KERNEL_SRC="linux-6.15.2"

COPY sources/$COMPILER_SRC.tar.xz /tmp
COPY sources/$KERNEL_SRC.tar.xz /tmp
COPY sources/$BUILDROOT_SRC.tar.xz /tmp
COPY configs/aarch64-softmmu_kernel_defconfig /tmp
COPY configs/aarch64-softmmu_buildroot_defconfig /tmp

# --- Cross compiler setup
RUN cd /tmp && \
    mkdir -p $WORKSPACE && \
    tar -xf $COMPILER_SRC.tar.xz -C $WORKSPACE

ENV COMPILER_HOME="$WORKSPACE/$COMPILER_SRC"
ENV PATH="$COMPILER_HOME/bin:$PATH"
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-none-linux-gnu-

# --- Buildroot
RUN cd /tmp && \
    tar -xf $BUILDROOT_SRC.tar.xz -C $WORKSPACE  && \
    mkdir -p $WORKSPACE/$BUILDROOT_SRC/board/aarch64-softmmu && \
    cp $KERNEL_SRC.tar.xz                   $WORKSPACE/$BUILDROOT_SRC/board/aarch64-softmmu && \
    cp aarch64-softmmu_kernel_defconfig     $WORKSPACE/$BUILDROOT_SRC/board/aarch64-softmmu && \
    cp aarch64-softmmu_buildroot_defconfig  $WORKSPACE/$BUILDROOT_SRC/configs && \
    cd $WORKSPACE/$BUILDROOT_SRC && \
    make aarch64-softmmu_buildroot_defconfig && \
    make -j4

# --- Setup compile_commands.json
RUN cd $WORKSPACE/$BUILDROOT_SRC/output/build/linux-custom && \
    python3 scripts/clang-tools/gen_compile_commands.py

# --- Pulling together build artefacts from different images
FROM base_image AS qemu_workspace

ENV WORKSPACE="/ws"
ENV ARCH=arm64
ENV BUILDROOT_SRC="buildroot-2025.05"
ENV COMPILER_SRC="arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu"
ENV COMPILER_HOME="$WORKSPACE/$COMPILER_SRC"
ENV CROSS_COMPILE=aarch64-none-linux-gnu-
ENV KERNEL_SRC="linux-6.15.2"
ENV PATH="$COMPILER_HOME/bin:$PATH"

COPY sources/$COMPILER_SRC.tar.xz /tmp
RUN cd /tmp && \
    mkdir -p $WORKSPACE && \
    tar -xf $COMPILER_SRC.tar.xz -C $WORKSPACE

COPY --from=qemu_image      $WORKSPACE/qemu_v10.0.2/build/qemu-system-aarch64       $WORKSPACE/qemu/
COPY --from=qemu_image      $WORKSPACE/qemu_v10.0.2/pc-bios/efi-virtio.rom          $WORKSPACE/qemu/
COPY --from=buildroot_image $WORKSPACE/$BUILDROOT_SRC/output/images/rootfs.ext3     $WORKSPACE/buildroot/output/images/
COPY --from=buildroot_image $WORKSPACE/$BUILDROOT_SRC/output/images/Image           $WORKSPACE/buildroot/output/images/
COPY --from=buildroot_image $WORKSPACE/$BUILDROOT_SRC/output/build/linux-custom     $WORKSPACE/buildroot/output/build/linux-custom
COPY configs/aarch64-softmmu_buildroot_defconfig                                    $WORKSPACE/buildroot/configs
COPY configs/aarch64-softmmu_kernel_defconfig                                       $WORKSPACE/buildroot/board/aarch64-softmmu
COPY run_qemu.sh                                                                    $WORKSPACE/qemu

# --- Clean
RUN apt-get clean && \
    rm -rf /tmp/* /var/tmp/*

CMD ["/bin/bash"]
