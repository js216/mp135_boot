BUILDDIR   = build
BINARYNAME = $(BUILDDIR)/main
OBJDIR     = $(BUILDDIR)/obj
OBJECTS    = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
LINKSCR    = stm32mp13xx_a7_sysram.ld

SOURCES = \
	 Src/main.c \
	 Drivers/syscalls.c \
	 Drivers/mmu_stm32mp13xx.c \
	 Drivers/system_stm32mp13xx_A7.c \
	 Drivers/startup_stm32mp135fxx_ca7.c \
	 Drivers/CMSIS/Core_A/Source/irq_ctrl_gic.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_ddr.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_dma.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_dma_ex.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_exti.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_gpio.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_i2c.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_rcc.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_rcc_ex.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_uart.c \
	 Drivers/STM32MP13xx_HAL_Driver/Src/stm32mp13xx_hal_uart_ex.c \
	 Drivers/STM32MP13xx_DISCO/stm32mp13xx_disco.c \
	 Drivers/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.c \
	 Drivers/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.c \

INCLUDES = \
	 -Isrc \
	 -IDrivers/CMSIS/Core_A/Include \
	 -IDrivers/CMSIS/Device/ST/STM32MP13xx/Include \
	 -IDrivers/STM32MP13xx_HAL_Driver/Inc \
	 -IDrivers/STM32MP13xx_DISCO \

CFLAGS = \
	 -g2 \
	 -O0 \
	 -fno-common \
	 -DUSE_FULL_LL_DRIVER \
	 -DSTM32MP135Fxx \
	 -DSTM32MP1 \
	 -DCORE_CA7 \
	 -mcpu=cortex-a7 -march=armv7ve -mfpu=neon-vfpv4 -mlittle-endian -mfloat-abi=hard \
	 $(INCLUDES) \
	 -fdata-sections -ffunction-sections \
	 -nostartfiles \
	 -ffreestanding \
	 -DDDR_TYPE_DDR3_4Gb \

LFLAGS = \
	 -Wl,--gc-sections \
	 -Wl,-Map,$(BINARYNAME).map,--cref \
	 -mcpu=cortex-a7 -march=armv7ve -mfpu=neon-vfpv4 -mlittle-endian -mfloat-abi=hard \
	 -T $(LINKSCR) \
	 -specs=nano.specs -specs=nosys.specs \
	 -nostartfiles \
	 -ffreestanding \
	 -Wl,--print-memory-usage \

.PHONY: all clean install read

all: $(BINARYNAME).stm32

clean:
	rm -rf build

install: $(BINARYNAME).stm32
	python3 scripts/uart_boot.py -c ASRL14::INSTR -f $<

read:
	python3 scripts/uart_read.py -c ASRL14::INSTR

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	arm-none-eabi-gcc -c $(CFLAGS) $< -o $@

$(BINARYNAME).elf: $(OBJECTS) $(LINKSCR)
	arm-none-eabi-gcc $(LFLAGS) -o $@ $(OBJECTS)

$(BINARYNAME).bin: $(BINARYNAME).elf
	arm-none-eabi-objcopy -O binary $< $@

$(BINARYNAME).stm32: $(BINARYNAME).bin
	python3 scripts/stm32_header.py -e $(BINARYNAME).elf -b $< -o $@ -t .RESET
