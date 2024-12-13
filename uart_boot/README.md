# UART boot of STM32MP135

The Jupyter notebook in this directory can be used to upload arbitrary code to
the STM32MP135 via UART using the boot ROM embedded in the processor.

### Hardware setup

Set `BOOT[2:0] = 0b000` to force serial (USB/UART) boot, making sure that the
DFU USB-C (CN7, lower left side) is *not* connected. Press the black reset
button (B5) and observe the red LED (LD4, upper right side of the board) blink
at 5 Hz. Then follow the steps in the Jupyter notebook.

### Formatting the executable file

In the notebook, we use as the example executable the STM32DDRFW-UTIL utility
taken from the STMicroelectronics
[repository](https://github.com/STMicroelectronics/STM32DDRFW-UTIL/tree/main)),
since it is already tested and properly formatted for execution.

In general, a program on an Arm processor will typically be compiled into an ELF
object file, which needs to be processed into an executable file with an
included header. "Each binary image loaded by the ROM code needs to include a
specific STM32 header added on top of the binary data."
[[ST wiki: STM32 header for binary files](https://wiki.st.com/stm32mpu/wiki/STM32_header_for_binary_files)]

This can be done with a custom script, or the one provided in the using the
[STM32CubeMP13 Package](https://www.st.com/en/embedded-software/stm32cubemp13.html).
In the package (I am using version `STM32Cube_FW_MP13_V1.2.0`), navigate to
Utilities -> ImageHeader -> Python3.

Note: To obtain the BIN file from an STM32CubeIDE project, open project
properties and navigate to C/C++ Build -> Settings -> Tool Settings -> MCU/MPU
Post build outputs. Check the option "Convert to binary file (-O binary)".

### Using STM32CubeProgrammer

The same thing, and much more, can also be accomplished using the
[`STM32CubeProgrammer`](https://www.st.com/en/development-tools/stm32cubeprog.html)
using the following command:

       $ .\STM32_Programmer_CLI -c port=com14 -w  flash.tsv

assuming a properly formatted flash.tsv (see example provided in this directory).

### Author

Jakob Kastelic, Stanford Research Systems
