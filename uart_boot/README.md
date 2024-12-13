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

An alternative example provided is `blink.stm32`, compiled from the program in
the directory `blink_cubeide` and formatted in `stm32_header`.

In general, a program on an Arm processor will typically be compiled into an ELF
object file, which needs to be processed into an executable file with an
included header. See the `stm32_header` directory in this repository for an
example of how this is done.

### Using STM32CubeProgrammer

The same thing as this notebook, and much more, can also be accomplished using the
[`STM32CubeProgrammer`](https://www.st.com/en/development-tools/stm32cubeprog.html)
using the following command:

       $ .\STM32_Programmer_CLI -c port=com14 -w  flash.tsv

assuming a properly formatted flash.tsv (see example provided in this directory).

### Author

Jakob Kastelic, Stanford Research Systems
