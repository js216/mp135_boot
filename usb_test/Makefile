PORT = COM15

BUILDDIR   = build
BINARYNAME = $(BUILDDIR)/main
OBJDIR     = $(BUILDDIR)/obj
OBJECTS    = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
LINKSCR    = stm32mp13xx_a7_sysram.ld

SOURCES = \
	 drivers/syscalls.c \
	 drivers/startup_stm32mp135c_ca7.c \
	 drivers/stm32mp13xx_disco.c \
	 drivers/stm32mp13xx_disco_bus.c \
	 drivers/stm32mp13xx_disco_stpmic1.c \
	 drivers/irq_ctrl_gic.c \
	 drivers/mmu_stm32mp13xx.c \
	 drivers/stm32mp13xx_hal.c \
	 drivers/stm32mp13xx_hal_dma.c \
	 drivers/stm32mp13xx_hal_dma_ex.c \
	 drivers/stm32mp13xx_hal_gpio.c \
	 drivers/stm32mp13xx_hal_i2c.c \
	 drivers/stm32mp13xx_hal_pcd.c \
	 drivers/stm32mp13xx_hal_pcd_ex.c \
	 drivers/stm32mp13xx_hal_pwr.c \
	 drivers/stm32mp13xx_hal_pwr_ex.c \
	 drivers/stm32mp13xx_hal_rcc.c \
	 drivers/stm32mp13xx_hal_rcc_ex.c \
	 drivers/stm32mp13xx_hal_tim.c \
	 drivers/stm32mp13xx_hal_tim_ex.c \
	 drivers/stm32mp13xx_hal_uart.c \
	 drivers/stm32mp13xx_hal_uart_ex.c \
	 drivers/stm32mp13xx_ll_usb.c \
	 drivers/usbd_cdc.c \
	 drivers/usbd_core.c \
	 drivers/usbd_ctlreq.c \
	 drivers/usbd_ioreq.c \
	 src/main.c \
	 src/stm32mp13xx_hal_msp.c \
	 src/stm32mp13xx_it.c \
	 src/system_stm32mp13xx_A7.c \
	 src/usb_device.c \
	 src/usbd_cdc_if.c \
	 src/usbd_desc.c \
	 src/usbd_conf.c \

INCLUDES = \
	 -Isrc \
	 -Idrivers \

MACROS = \
	-DSTM32MP135Fxx \
	-DCORE_CA7 \
	-DCACHE_USE \
	-DMMU_USE \
	-DUSE_HAL_DRIVER \
	-DUSE_STM32MP13XX_DK \
	-DUSE_IOEXPANDER \
	-DUSE_USB_HS \
	-D__LOG_UART_IO_ \
	-DLOGLEVEL=LOGDBG \

CFLAGS = \
	 $(INCLUDES) \
	-mcpu=cortex-a7 \
	-std=gnu11 \
	-g3 \
	 $(MACROS) \
	-O0 \
	-ffunction-sections \
	-Wall \
	-Wno-strict-aliasing \
	-fstack-usage \
	--specs=nano.specs \
	-mfpu=vfpv4-d16 \
	-mfloat-abi=hard \
	-mthumb \

LFLAGS = \
	 -Wl,-Map,$(BINARYNAME).map,--cref \
	 -T$(LINKSCR) \
	 -Wl,--print-memory-usage \
	 -mcpu=cortex-a7 \
	 --specs=nosys.specs \
	 -Wl,--gc-sections \
	 -static \
	 --specs=nano.specs \
	 -mfpu=vfpv4-d16 \
	 -mfloat-abi=hard \
	 -mthumb \
	 -Wl,--start-group -lc -lm -Wl,--end-group \

.PHONY: all clean install check-port term

all: $(BINARYNAME).stm32

clean:
	rm -rf build

check-port:
ifndef PORT
	$(error PORT not set. Usage: make install PORT=COM1)
endif

term: check-port
	python3 -m serial.tools.miniterm $(PORT) 115200

install: $(BINARYNAME).stm32 check-port
	python3 scripts/uart_boot.py -c $(PORT) -f $<

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	arm-none-eabi-gcc -c $(CFLAGS) $< -o $@

$(BINARYNAME).elf: $(OBJECTS) $(LINKSCR)
	arm-none-eabi-gcc $(LFLAGS) -o $@ $(OBJECTS)

$(BINARYNAME).bin: $(BINARYNAME).elf
	arm-none-eabi-objcopy -O binary $< $@

$(BINARYNAME).stm32: $(BINARYNAME).bin
	python3 scripts/stm32_header.py -e $(BINARYNAME).elf -b $< -o $@ -t .RESET
