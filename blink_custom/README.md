# Blink without an IDE

This program runs on a custom STM32MP135F board, blinking the red LED
(via port PA13), and printing a "Hello World" message to UART4 (connected to
STLink and available via CN10, the Micro USB connector).

This program is a simplification of `blink_noide`, removing as much of the
unnecessary boilerplate as possible. It also removes STPMIC1 support, since the
custom board does not use it.

### Getting started

1. To compile the program, run Make from this directory:

       $ cd blink_custom
       $ make

2. To download the program to the board, run

       $ make install PORT=COM20

### Author

Jakob Kastelic, Stanford Research Systems
