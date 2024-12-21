# STPMIC1 Linux kernel driver with STM32MP135

In this example, we show how to reassign the STPMIC1 from the trusted execution
environment (OP-TEE) to the "untrusted" Linux kernel. In this way, the I2C4 can
be unsecured and used directly from the kernel and userspace.

Note that this is NOT supported by official ST resources, where the secure OS is
strongly recommended to be used for power management, using the SCMI interface.

### Prerequisites

This example assumes a running Buildroot-based configuration that boots Linux
and OP-TEE using TF-A, without U-Boot (see `tfa_falcon`).

### Steps to getting it working

1. In the OP-TEE device tree source, locate the line

    DECPROT(STM32MP1_ETZPC_I2C4_ID, DECPROT_S_RW, DECPROT_UNLOCK)

   and change `DECPROT_S_RW` to `DECPROT_NS_RW`. This configures the ETZPC
   peripheral to allow access to the I2C4 peripheral from the non-secure world,
   as well as the secure world. Read more about the ETZPC in the STM32MP13xx
   reference manual, the first couple pages of the ETZPC chapter are short and
   easy to understand.

   Note that with that change, OP-TEE will no longer work since (without further
   changes or additions) it will refuse to talk to a non-secured peripheral.
   This is not a problem, since the whole point of this example is to remove
   I2C4 from OP-TEE's control.

2. Move the entire `&i2c4 {...}` tree from the OP-TEE device tree source to the
   Linux DTS (about 180 lines of code!). OP-TEE will not need to know about
   I2C4, but Linux will.

3. Notice that the `$i2c4` tree refers to `pinctrl-0 = <&i2c4_pins_a>`, which is
   not defined by default. Without it, the driver will not know what pins we're
   using for the I2C4 peripheral. Copy the pin mux definition to the appropriate
   Linux DTS file (`arch/arm/boot/dts/stm32mp13-pinctrl.dtsi`) from the
   corresponding OP-TEE file (`core/arch/arm/dts/stm32mp13-pinctrl.dtsi`):

       i2c4_pins_a: i2c4-0 {
       	pins {
       		pinmux = <STM32_PINMUX('E', 15, AF6)>, /* I2C4_SCL */
       			 <STM32_PINMUX('B', 9, AF6)>; /* I2C4_SDA */
       		bias-disable;
       		drive-open-drain;
       		slew-rate = <0>;
       	};
       };

4. Remove all references to voltage regulators from the OP-TEE DTS.

5. Behind the scenes, two more things refer to regulators in the OP-TEE files.
   In `core/arch/arm/dts/stm32mp131.dtsi`, remove the following lines:

       		operating-points-v2 = <&cpu0_opp_table>;

       [...]

       cpu0_opp_table: cpu0-opp-table {
       	compatible = "operating-points-v2";
       };

       [...]

       sdmmc1_io: sdmmc1_io {
       	compatible = "st,stm32mp13-iod";
       	regulator-name = "sdmmc1_io";
       	regulator-always-on;
       };

       sdmmc2_io: sdmmc2_io {
       	compatible = "st,stm32mp13-iod";
       	regulator-name = "sdmmc2_io";
       	regulator-always-on;
       };

   In `core/arch/arm/dts/stm32mp13xd.dtsi`, remove the following (which is the
   entirety of the file):

       &cpu0_opp_table {
       		opp-1000000000 {
       			opp-hz = /bits/ 64 <1000000000>;
       			opp-microvolt = <1350000>;
       			opp-supported-hw = <0x2>;
       			st,opp-default;
       		};

       		opp-650000000 {
       			opp-hz = /bits/ 64 <650000000>;
       			opp-microvolt = <1250000>;
       			opp-supported-hw = <0x2>;
       		};
       };

6. The Linux driver for the STPMIC1 requires two small changes. In
   `drivers/regulator/stpmic1_regulator.c`, we need to tell it that the switched
   regulator (`pwr_sw2` in the device tree, and `3V3_SW` on the STM32MP135F-DK
   schematic) is 3.3V, not 5V. Locate the lines starting with `#define
   REG_SW_OUT(ids, base) { \` and change the following `.fixed_uV = 5000000` to
   `.fixed_uV = 3300000`.

   The second change is in file `drivers/mfd/stpmic1.c`. In function
   `stpmic1_probe()`, comment out (or remove) the lines

       if (ddata->irq < 0) {
       	dev_err(dev, "Failed to get main IRQ: %d\n", ddata->irq);
       	return ddata->irq;
       }

   and also

       /* Initialize PMIC IRQ Chip & associated IRQ domains */
       ret = devm_regmap_add_irq_chip(dev, ddata->regmap, ddata->irq,
       			       IRQF_ONESHOT | IRQF_SHARED,
       			       0, &stpmic1_regmap_irq_chip,
       			       &ddata->irq_data);
       if (ret) {
       	dev_err(dev, "IRQ Chip registration failed: %d\n", ret);
       	return ret;
       }

### Author

Jakob Kastelic, Stanford Research Systems
