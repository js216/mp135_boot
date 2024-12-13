# Blink without an IDE

This program runs on the STM32MP135F-DK evaluation board, blinking the red LED
(via port PA13), and printing a "Hello World" message to UART4 (connected to
STLink and available via CN10, the Micro USB connector).

This program is a simplification of `blink_cubeide`, removing as much of the
unnecessary boilerplate as possible.

It is built by a single Makefile without external dependencies. The first part
of the Makefile defines all the source code files used in the project. The
middle part defines the compiler and linker flags. Finally, the last part
defines the recipes for the build process:

1. Create object (`.o`) files from source code (`.c`).
2. Link object files into a single executable (`.elf`).
3. Extract the binary bytes (`.bin`) from the executable.
4. Prepend the STM32 header (`.stm32`); see `stm32_header` for details.
5. Install the exe to the processor via UART; see `uart_boot` for details.

### Getting started

1. To compile the program, run Make from the build directory:

       $ cd blink_noide/build
       $ make

2. To download the program to the board, run

       $ make install

See `uart_boot` for details on how to configure the hardware to accept the
program bitstream over UART.

### Author

Jakob Kastelic, Stanford Research Systems
