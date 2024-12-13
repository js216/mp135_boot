# STM32 header for STM32MP135 programs

the Jupyter notebook in this directory can be used to convert an ELF object file
to an executable with the correct header file required by the boot ROM.

In general, a program on an Arm processor will typically be compiled into an ELF
object file, which needs to be processed into an executable file with an
included header. "Each binary image loaded by the ROM code needs to include a
specific STM32 header added on top of the binary data."
[[ST wiki: STM32 header for binary files](https://wiki.st.com/stm32mpu/wiki/STM32_header_for_binary_files)]

The
[STM32CubeMP13 Package](https://www.st.com/en/embedded-software/stm32cubemp13.html)
provides a utility to create the correct header.
In the package (I am using version `STM32Cube_FW_MP13_V1.2.0`), navigate to
Utilities -> ImageHeader -> Python3.

To obtain a BIN file from an STM32CubeIDE project, open project properties and
navigate to C/C++ Build -> Settings -> Tool Settings -> MCU/MPU Post build
outputs. Check the option "Convert to binary file (-O binary)".

Follow the steps in the Jupyter notebook to convert an ELF file into an
executable with the correct STM32 header.

### Author

Jakob Kastelic, Stanford Research Systems
