#!/bin/bash

echo "---------------------"
echo "  Download sources   "
echo "---------------------"

CURRENT_DIR=$(pwd)
TARGET_DIR=${CURRENT_DIR}/sourcess

function download_n_verify() 
{
    local file=$1
    local link=$2
    local sum=$3

    echo ""
    echo "*** ${file} ***"
    if ! [ -f ${TARGET_DIR}/${file} ]; then
        wget -P ${TARGET_DIR} ${link}
    fi
    echo "${sum} ${TARGET_DIR}/${file}" | sha256sum --check 
}

COMPILER_FILE="arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz"
COMPILER_LINK="https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz"
COMPILER_SUM="47aeefc02b0ee39f6d4d1812110952975542d365872a7474b5306924bca4faa1"

KERNEL_FILE="linux-6.15.2.tar.xz"
KERNEL_LINK="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.15.2.tar.xz"
KERNEL_SUM="3458cd6a6c508e161dbc5406e72b99d5dbdf929faf704a67db9ba46d07514858"

BUILDROOT_FILE="buildroot-2025.05.tar.xz"
BUILDROOT_LINK="https://buildroot.org/downloads/buildroot-2025.05.tar.xz"
BUILDROOT_SUM="3c93b9b0a2dcec4ce304b6917b371e6d805776b9bc49655c3e89d9455e4b0ec4"

mkdir -p ${TARGET_DIR}
download_n_verify ${COMPILER_FILE} ${COMPILER_LINK} ${COMPILER_SUM}
download_n_verify ${KERNEL_FILE} ${KERNEL_LINK} ${KERNEL_SUM}
download_n_verify ${BUILDROOT_FILE} ${BUILDROOT_LINK} ${BUILDROOT_SUM}
