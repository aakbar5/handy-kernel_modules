# !/bin/bash

export WORKSPACE="/ws"
export BUILDROOT_HOME="${WORKSPACE}/buildroot-2019.08.3"
export LINUX_HOME="${WORKSPACE}/linux-5.5"
export QEMU_HOME="${WORKSPACE}/qemu-4.2.0"
export COMPILER_HOME="${WORKSPACE}/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu"

echo "===== Launch QEMU"
${QEMU_HOME}/aarch64-softmmu/qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a57 \
  -nographic \
  -smp 2 \
  -m 3072 \
  -drive format=raw,file=${BUILDROOT_HOME}/output/images/rootfs.ext3 \
  -kernel ${LINUX_HOME}/arch/arm64/boot/Image \
  -append "console=ttyAMA0 root=/dev/vda oops=panic panic_on_warn=1 panic=-1 ftrace_dump_on_oops=orig_cpu debug earlyprintk=serial slub_debug=UZ ip=dhcp" \
  -nic user,hostfwd=tcp::2222-:22
