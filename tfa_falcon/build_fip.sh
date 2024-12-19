#!/bin/bash

# known starting point
cd arm-trusted-firmware
rm -rf build
git reset --hard

# apply patch to increase max allowed size of BL33
patch -p1 < ../bl2.patch

# compile with lots of options
/usr/bin/make -j25 \
   CROSS_COMPILE="arm-linux-gnueabi-" \
   BUILD_STRING=custom \
   STM32MP_SDMMC=1 \
   DTB_FILE_NAME=stm32mp135f-dk.dtb \
   STM32MP_USB_PROGRAMMER=1 \
   PLAT=stm32mp1 \
   ARM_ARCH_MAJOR=7 \
   ARCH=aarch32 \
   AARCH32_SP=optee \
   BL32=/home/jk/srs_os/build/images/tee-header_v2.bin \
   BL32_EXTRA1=/home/jk/srs_os/build/images/tee-pager_v2.bin \
   BL32_EXTRA2=/home/jk/srs_os/build/images/tee-pageable_v2.bin \
   BL33_CFG=/home/jk/srs_os/build/images/stm32mp135f-dk.dtb \
   BL33=/home/jk/srs_os/build/build/linux-custom/arch/arm/boot/zImage \
   all fip
