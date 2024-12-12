# Blink in STM32CubeIDE

This program runs on the STM32MP135F-DK evaluation board, blinking the red LED
(via port PA13), and printing a "Hello World" message to UART4 (connected to
STLink and available via CN10, the Micro USB connector).

It has been extracted from the [STM32CubeMP13
Package](https://www.st.com/en/embedded-software/stm32cubemp13.html) (see also
the [ST wiki](https://wiki.st.com/stm32mpu/wiki/STM32CubeMP13_Package_-_Getting_started)),
removing the dependencies on files external to the IDE project. (The ST package
embeds these external dependencies as Eclipse "Linked Resources", making it
unclear which files are actually necessary.)

Three sets of drivers, taken from the ST package, are included with this example:

- `CMSIS`: a minimal set of CMSIS drivers, defining device registers (in
  `stm32mp135fxx_ca7.h`) and similar
- `STM32MP13xx_HAL_Driver`: ST hardware abstraction layer for a number of
  peripherals (in this example we only use UART and GPIO)
- `STM32MP13xx_DISCO`: a minimal set of drivers for the evaluation board
  components (significantly, the power management IC)

Note that of the 15 MB of code included in this project, 99.9992% of it is
boilerplate. The only "application logic" are the following lines in `main.c`:

    int i = 0;
    while(1) {
        printf("Hello World %d\r\n", i++);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_13);
        HAL_Delay(1000);
    }

### Getting started

Hardware setup: power the STM32MP135F-DK board by connecting USB-C into CN12
(right of the LCD), and connect a Micro USB cable to CN10 (left of LCD). Setup
the DIP switch (SW1) so that `BOOT[2:0] = 0b100` (test/engineering boot), which
allows the STLink to act as a debugger, and press the black reset pushbutton
(B5).

1. Open a serial monitor on the STLink.

2. Import the project into Eclipse via File -> Open Projects from File System...

3. Right click on the project in Project Explorer and select Build Project.

4. Right click on the project in Project Explorer and select Run As -> Run
   Configurations ...

5. Double click on "STM32 C/C++ Application", and then press Run.

The program should load into SYSRAM, the red LED next to the two blue pushbutton
(upper right of the LCD) should be blinking at 1 Hz, and the serial monitor
should read a "Hello World" message every second.

The following message gets printed in the STM32CubeIDE console:

       Open On-Chip Debugger 0.12.0+dev-00600-g090b431b1 (2024-09-13-19:14) [https://github.com/STMicroelectronics/OpenOCD]
       Licensed under GNU GPL v2
       For bug reports, read
       	http://openocd.org/doc/doxygen/bugs.html
       srst_only srst_pulls_trst srst_gates_jtag srst_open_drain connect_deassert_srst
       Info : Listening on port 6666 for tcl connections
       Info : Listening on port 4444 for telnet connections
       Info : STLINK V3J15M6B5S1 (API v3) VID:PID 0483:3753
       Info : Target voltage: 3.294430
       Info : Unable to match requested speed 5000 kHz, using 3300 kHz
       Info : Unable to match requested speed 5000 kHz, using 3300 kHz
       Info : clock speed 3300 kHz
       Info : stlink_dap_op_connect(connect)
       Info : SWD DPIDR 0x6ba02477
       Info : stlink_dap_op_connect(connect)
       Info : SWD DPIDR 0x6ba02477
       Info : [STM32MP135FAFx.ap1] Examination succeed
       Info : [STM32MP135FAFx.axi] Examination succeed
       Info : STM32MP135FAFx.cpu: hardware has 6 breakpoints, 4 watchpoints
       Info : [STM32MP135FAFx.cpu] Examination succeed
       Info : gdb port disabled
       Info : gdb port disabled
       Info : starting gdb server for STM32MP135FAFx.cpu on 3333
       Info : Listening on port 3333 for gdb connections
       Info : accepting 'gdb' connection on tcp/3333
       Info : STM32MP135FAFx.cpu: MPIDR level2 0, cluster 0, core 0, multi core, no SMT
       target halted in Thumb state due to debug-request, current mode: System
       cpsr: 0x2000013f pc: 0x2ffed408
       MMU: disabled, D-Cache: disabled, I-Cache: disabled
       Info : New GDB Connection: 1, Target STM32MP135FAFx.cpu, state: halted
       Info : stlink_dap_op_connect(connect)
       Info : SWD DPIDR 0x6ba02477
       Info : stlink_dap_op_connect(connect)
       Info : SWD DPIDR 0x6ba02477
       target halted in Thumb state due to debug-request, current mode: Supervisor
       cpsr: 0x200001f3 pc: 0x00004904
       MMU: disabled, D-Cache: disabled, I-Cache: disabled
       shutdown command invoked
       Info : dropped 'gdb' connection

### Author

Jakob Kastelic, Stanford Research Systems
