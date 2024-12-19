# "Falcon mode" using TF-A

In this example, we use Arm Trusted Firmware (TF-A) to load the Linux kernel
directly, without using U-Boot.

This approach is inspired by the ST wiki article
[How to optimize the boot time](https://wiki.st.com/stm32mpu/wiki/How_to_optimize_the_boot_time),
under "Optimizing boot-time by removing U-Boot".

There are two ways to get started, either the manual method, or using the
provided script.

### Prerequisites

This example assumes OP-TEE and Linux kernel have been configured and compiled
correctly. After the kernel is started, it will also require a root filesystem.

### Manual method

1. Obtain source code for the Arm Trusted firmware:

       $ git clone git@github.com:STMicroelectronics/arm-trusted-firmware.git

2. Two files need to be modified. Since the kernel is much bigger than U-Boot,
   it takes longer to load. We need to adjust the SD card reading timeout. In
   `drivers/st/mmc/stm32_sdmmc2.c`, find the line

       timeout = timeout_init_us(TIMEOUT_US_10_MS);

   and replace it with

       timeout = timeout_init_us(TIMEOUT_US_10_MS * 5);

   Next, we would like to load the kernel deep enough into the memory space so
   that relocation of the compressed image is not necessary. In file
   `plat/st/stm32mp1/stm32mp1_def.h`, find the line

       #define STM32MP_BL33_BASE              STM32MP_DDR_BASE

   and replace it with

       #define STM32MP_BL33_BASE              (STM32MP_DDR_BASE + U(0x2008000))

   Finally, in order to allow loading such a big `BL33` as the kernel image, we
   adjust the max size. In the same file, find the line

       #define STM32MP_BL33_MAX_SIZE          U(0x400000)

   and replace it with

       #define STM32MP_BL33_MAX_SIZE          U(0x3FF8000)

3. Build TF-A:

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

   Be sure to adjust `BL32`, `BL32_EXTRA1`, `BL32_EXTRA2`, `BL33_CFG`, and
   `BL33` to point to the three OP-TEE binaries, the DTB for the kernel, and the
   kernel itself (see prerequisites).

5. The finished binaries are located under
   `arm-trusted-firmware/build/stm32mp1/release`. The two files we need from
   here are the TF-A itself (`tf-a-stm32mp135f-dk.stm32`) and the firmware
   package containing OP-TEE and the Linux kernel (`fip.bin`).

   The TF-A executable needs to be written to the first two partitions of the SD
   card, followed by `fip.bin` in the third. The fourth partition is the root
   filesystem used by the Linux kernel after it boots. (How to create an SD card
   image is beyond the scope of this example.) For example, we can have the
   following layout:

       $ fdisk -l sdcard.img
       Disk sdcard.img: 58 MiB, 60813312 bytes, 118776 sectors
       Units: sectors of 1 * 512 = 512 bytes
       Sector size (logical/physical): 512 bytes / 512 bytes
       I/O size (minimum/optimal): 512 bytes / 512 bytes
       Disklabel type: gpt
       Disk identifier: AFADA8B1-B30C-40ED-A9F8-18513820EC2D

       Device      Start    End Sectors  Size Type
       sdcard.img1    34    207     174   87K Linux filesystem
       sdcard.img2   208    381     174   87K Linux filesystem
       sdcard.img3   382  16336   15955  7.8M Linux filesystem
       sdcard.img4 16337 118736  102400   50M Linux filesystem

### Using the script

Instead of carrying out the above steps manually, we can also use the script
`build_fip.sh` provided in this directory. The steps are as follows:

1. As before, obtain source code for the Arm Trusted firmware:

       $ git clone git@github.com:STMicroelectronics/arm-trusted-firmware.git

2. Open the script in a text editor and examine what it does, confirming it is
   equivalent to the manual steps above.

3. In `build_fip.sh`, modify the variables `BL32`, `BL32_EXTRA1`, `BL32_EXTRA2`,
   `BL33_CFG`, and `BL33` to point to the three OP-TEE binaries, the DTB for the
   kernel, and the kernel itself.

4. Run the script:

       $ cd tfa_falcon
       $ ./build_fip.sh

5. Load the files to the SD card as in the manual method.

### Notes for Buildroot

Buildroot can also be used to build Arm Trusted Firmware. In that case, we need
to arrange that it receives the correct `BL33` parameters. For example, we can
add the following configuration option to
`buildroot/boot/arm-trusted-firmware/Config.in`:

    config BR2_TARGET_ARM_TRUSTED_FIRMWARE_LINUX_AS_BL33
    	bool "Linux kernel"
    	depends on BR2_LINUX_KERNEL
    	help
    	  This option allows to embed the Linux kernel as the BL33 part of the
    	  ARM Trusted Firmware.

    	  Do not choose this option if you intend to use U-Boot or another
    	  second-stage bootloader. With this option, TF-A starts Linux directly.

    	  With this option chosen, whenever the Linux zImage changes, TF-A needs
    	  to be re-built. Since Buildroot does not track package dependencies,
    	  this has to be done manually by invoking
    	  `make arm-trusted-firmware-rebuild`.

To implement the effect of this configuration option, add the following to
`buildroot/boot/arm-trusted-firmware/arm-trusted-firmware.mk`:

    ifeq ($(BR2_TARGET_ARM_TRUSTED_FIRMWARE_LINUX_AS_BL33),y)
    ARM_TRUSTED_FIRMWARE_MAKE_OPTS += BL33=$(BINARIES_DIR)/zImage
    ARM_TRUSTED_FIRMWARE_MAKE_OPTS += BL33_CFG=$(BINARIES_DIR)/$(LINUX_DTBS)
    ARM_TRUSTED_FIRMWARE_DEPENDENCIES += linux
    endif

Finally, the patch that modifies the TF-A source code (step 2 of the manual
method, above) needs to be in a location where Buildroot can find it. To this
end, define `BR2_GLOBAL_PATCH_DIR` and place the patch file there, i.e., to the
location

    $(BR2_GLOBAL_PATCH_DIR)/arm-trusted-firmware/0001-allow-large-bl33.patch

### Author

Jakob Kastelic, Stanford Research Systems
