# DDR memory test

This program runs on the STM32MP135F-DK evaluation board, initializing the DDR3L
memory, then writing a sequence of pseudo-random bits to the memory and reading
them back to verify if they were correctly written.

This program is adapted from the `DDR_Init` example from the `STM32CubeMP13`
package provided by ST.

### Getting started

1. To compile the program, run Make from the build directory:

       $ cd blink_noide/build
       $ make

2. To download the program to the board, run

       $ make install

   See `uart_boot` for details on how to configure the hardware to accept the
   program bitstream over UART.

3. Monitor the UART for messages about the status of the memory test.

### Author

Jakob Kastelic, Stanford Research Systems
