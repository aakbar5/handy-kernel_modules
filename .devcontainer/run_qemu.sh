# !/bin/bash

export WORKSPACE="/ws/"
export QEMU_HOME="${WORKSPACE}/qemu"
export BUILD_ROOT_HOME="${WORKSPACE}/buildroot"
export BUILD_ROOT_OUTPUT="${BUILD_ROOT_HOME}/output/images"
export ARCH=arm64
export CROSS_COMPILE=aarch64-none-linux-gnu-

echo "===== Launch QEMU"
${QEMU_HOME}/qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a57 \
  -nographic \
  -smp 2 \
  -m 3072 \
  -L ${QEMU_HOME} \
  -drive format=raw,file=${BUILD_ROOT_OUTPUT}/rootfs.ext3 \
  -kernel ${BUILD_ROOT_OUTPUT}/Image \
  -append "console=ttyAMA0 root=/dev/vda oops=panic panic_on_warn=1 panic=-1 ftrace_dump_on_oops=orig_cpu debug earlyprintk=serial slub_debug=UZ ip=dhcp" \
  -nic user,hostfwd=tcp::2222-:22

