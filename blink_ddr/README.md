# Blink running in DDR

This program runs on the STM32MP135F-DK evaluation board, blinking the red LED
(via port PA13), and printing a "Hello World" message to UART4 (connected to
STLink and available via CN10, the Micro USB connector).

This program is adapted from `blink_noide`, but linked to run out of DDR. To do
so, we change the linker script. In file `stm32mp13xx_a7_sysram.ld`, the
statement `REGION_ALIAS("RAM", SYSRAM_BASE)` defines RAM to be SYSRAM. We change
it to

    REGION_ALIAS("RAM", DDR_BASE);

where the two base addresses are given as:

    MEMORY {
          SYSRAM_BASE (rwx)   : ORIGIN = 0x2FFE0000, LENGTH = 128K
          DDR_BASE (rwx)      : ORIGIN = 0xC0000000, LENGTH = 512M
    }

### Getting started

1. To compile the program, run Make from this directory:

       $ cd blink_ddr
       $ make

   Verify that the linker reports that no SYSRAM (or SRAM) has been used, just
   DDR. (The following information is reporten when `gcc` is given the flags
   `-Wl,--print-memory-usage`).

       Memory region         Used Size  Region Size  %age Used
            SYSRAM_BASE:          0 GB       128 KB      0.00%
             SRAM1_BASE:          0 GB        16 KB      0.00%
             SRAM2_BASE:          0 GB         8 KB      0.00%
             SRAM3_BASE:          0 GB         8 KB      0.00%
               DDR_BASE:      103936 B       512 MB      0.02%

   Amongst other information, `stm32_header.py` prints the following:

       Image Size  : 67228 bytes
       Image Load  : 0xC0000000

   These two values need to be known to (or deduced by) the bootloader, such as
   the one in the example `sd_to_ddr`. (The load address can be easily changed
   by modifying the linker script to provide an alternative `ORIGIN`.)

2. Use some other tool to copy the program (`main.bin`) to the SD card, in any
   location. Note the size of the `.bin` executable (should be the same as
   reported in step 1.) and where it is located on the SD card.

3. See `sd_to_ddr` as an example bootloader that copies the executable from the
   SD card to DDR memory, and executes it there.

### Author

Jakob Kastelic, Stanford Research Systems
