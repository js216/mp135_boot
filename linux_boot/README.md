## WORK IN PROGRESS! This example does not yet work!

# "Bare metal" Linux bootloader on STM32MP1

This program runs on the STM32MP135F-DK evaluation board, initializing the DDR3L
memory and the SD card. The program then loads the DTB ("device tree blob") and
the compressed kernel image (`zImage`) to DDR and passes control to the kernel.

As such, this is the simplest possible bootloader. Please note that ST does not
recommend nor support this method of booting Linux on the STM32MP135 processors.
The recommended method is to use Arm Trusted Firmware (TF-A) to boot OP-TEE and
start U-Boot. The latter then loads and executes the Linux kernel.

### Getting started

Pre-requisites: compile a Linux kernel and the device tree. This results in two
files, which we will call `zImage` and `stm32mp135f-dk.dtb`.

1. Create an SD card image containing with two partitions, then note the offset
   where they are located. For example:

       $ dd if=/dev/zero of=sd.img bs=10M count=1
       $ cfdisk sd.img

   Select label type `gpt`, and create two partitions, one 64K and one taking up
   the rest of the free space. Write the partition to disk and remember where
   the DTB and the kernel are to be located:

       $ fdisk -l sd.img

       Disk sd.img: 10 MiB, 10485760 bytes, 20480 sectors
       Units: sectors of 1 * 512 = 512 bytes
       Sector size (logical/physical): 512 bytes / 512 bytes
       I/O size (minimum/optimal): 512 bytes / 512 bytes
       Disklabel type: gpt
       Disk identifier: F02F1C18-023A-2749-8C22-D7AEEF70C914

       Device     Start   End Sectors Size Type
       sd.img1     2048  2175     128  64K Linux filesystem
       sd.img2     4096 20446   16351   8M Linux filesystem

   In this example, we will place the DTB in first partition (sector 2048) and
   the kernel image to the second one (sector 4096).

2. Setup the two partitions as virtual devices and copy the DTB and kernel to
   them:

       $ sudo losetup -o $((512*2048)) /dev/loop0 sd.img
       $ sudo dd if=stm32mp135f-dk.dtb of=/dev/loop0
       $ sudo losetup --detach /dev/loop0

       $ sudo losetup -o $((512*4096)) /dev/loop0 sd.img
       $ sudo dd if=zImage of=/dev/loop0
       $ sudo losetup --detach /dev/loop0

3. Copy the `sd.img` image to the SD card, for example:

       $ sudo dd if=sd.img of=/dev/sdb

   Be *very sure* that `/dev/sdb` is the SD card, since all data on that device
   will be destroyed and replaced by the kernel image and DTB.

2. To compile the program, run Make from the build directory:

       $ cd sd_to_ddr/build
       $ make

3. To download the program to the board, run

       $ make install

   See `uart_boot` for details on how to configure the hardware to accept the
   program bitstream over UART.

4. Monitor the UART for messages about the status of the memory test.

### Author

Jakob Kastelic, Stanford Research Systems
