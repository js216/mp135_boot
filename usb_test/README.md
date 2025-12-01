# USB test

This program runs on the STM32MP135F-DK evaluation board, initializing the USB
hardware, then setting up the USB device functionality.

This program is adapted from the `CDC_Standalone` example from the
`STM32CubeMP13` package provided by ST.

### Getting started

1. To compile the program, run Make from this directory:

       $ cd usb_test
       $ make

2. To download the program to the board and monitor UART messages, run

       $ make install term

### Author

Jakob Kastelic, Stanford Research Systems
