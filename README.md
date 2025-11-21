# STM32MP135 Boot Experiments

This repo contains simple code examples booting code on the
[STM32MP135](https://www.st.com/en/microcontrollers-microprocessors/stm32mp135.html)
processors, as implemented on the official [eval
board](https://www.st.com/en/evaluation-tools/stm32mp135f-dk.html).

The official documentation and code examples by ST is extensive, but in many
cases overly complicated. These examples aim to be self-contained minimum
working examples.

The files are as follows:

- `blink_cubeide`: "Blink" program done as an STM32CubeIDE project and uploaded
  to the processor using the on-board STLink debugger in "test boot" mode

- `stm32_header`: Jupyter notebook to convert an ELF object file to an
  executable with the correct header file required by the boot ROM

- `uart_boot`: Jupyter notebook implementing all the commands used by the device
  boot ROM to write an executable to the internal SRAM of the device

- `blink_noide`: "Blink" program done without an IDE, compiled using arm-gcc
  with a simple Makefile

- `blink_custom`: Same as `blink_noide`, but simplified for a custom board

- `ddr_test`: Initialize DDR3L memory, fill it with pseudorandom bits, and
  confirm that reading from the memory returns the same bit sequence

- `sd_to_ddr`: Initialize SD card and DDR memory, then copy program from SD to
  DDR and execute it

- `blink_ddr`: Same as `blink_noide`, but linked to run out of RAM

- `linux_boot`: (Work in progress!) Load a DTB and compressed kernel image to
  DDR and run it

- `tfa_falcon`: Use Arm Trusted Firmware (TF-A) to load the Linux kernel without
  U-Boot, a variant of "falcon mode" boot

- `remove_optee`: Remove OP-TEE, reallocating STPMIC1 control to the Linux
  kernel driver, and clock/ETZPC/TZC configuration to TF-A.

### Author

Jakob Kastelic, Stanford Research Systems
