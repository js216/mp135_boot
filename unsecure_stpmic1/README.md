# STPMIC1 Linux kernel driver with STM32MP135

In this example, we show how to reassign the STPMIC1 from the trusted execution
environment (OP-TEE) to the "untrusted" Linux kernel. In this way, the I2C4 can
be unsecured and used directly from the kernel and userspace.

Later, we show how to configure all clocks from Arm Trusted Firmware (TF-A), so
that OP-TEE does not have to be used for this purpose.

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
   I2C4 and STPMIC1, but Linux will.

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

4. Remove all references to voltage regulators from the OP-TEE DTS. In
   particular, search for `scmi` and remove dependencies on anything SCMI.

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

7. Remove all references to SCMI from the Linux DTS, making sure they are
   replaced with corresponding references to the STPMIC1.

### Further cleanup

1. From the OP-TEE DTS, we can remove the lines

       vin: vin {
       	compatible = "regulator-fixed";
       	regulator-name = "vin";
       	regulator-min-microvolt = <5000000>;
       	regulator-max-microvolt = <5000000>;
       	regulator-always-on;
       };

       vdd: vdd {
       	compatible = "regulator-fixed";
       	regulator-name = "vdd";
       	regulator-min-microvolt = <3300000>;
       	regulator-max-microvolt = <3300000>;
       	regulator-always-on;
       };

       v3v3_ao: v3v3_ao {
       	compatible = "regulator-fixed";
       	regulator-name = "v3v3_ao";
       	regulator-min-microvolt = <3300000>;
       	regulator-max-microvolt = <3300000>;
       	regulator-always-on;
       };

   Now OP-TEE will complain that "VDD regulator not found". So we also need to
   remove the part that requires the regulator. In
   `core/arch/arm/plat-stm32mp1/drivers`, comment out the body of the function
   `stm32mp_syscfg_set_hslv()`, except for `return TEE_SUCCESS;`. This function
   is not needed so long as we do not intend to use the HSLV mode, i.e., OTP18
   bit 13 is not set, and VDD is not below 2.5 V.

2. Remove `serial1 = &usart1;` from the `aliases { ... }` section.

3. Remove the following sections from `\ { ... }`:

         magic_wol: magic_wol {
         	compatible = "st,stm32mp1,pwr-irq-user";
         	st,wakeup-pin-number = <2>;
         	status = "okay";
         };

         typec_wakeup: typec_wakeup {
         	compatible = "st,stm32mp1,pwr-irq-user";
         	/* Alert pin on PI2, wakeup_pin_5 */
         	st,wakeup-pin-number = <5>;
         	st,notif-it-id = <1>;
         	status = "okay";
         };

4. Remove the following sections:

       &gpiob {
       	st,protreg = < (TZPROT(9)) >;
       };

       &gpiod {
       	st,protreg = < (TZPROT(7)) >;
       };

       &gpioe {
       	st,protreg = < (TZPROT(15)) >;
       };

       &gpioi {
       	st,protreg = < (TZPROT(2)|TZPROT(3)) >;
       };

       &hash {
       	status = "okay";
       };

       &hse_monitor {
       	status = "okay";
       };

       &iwdg1 {
       	timeout-sec = <32>;
       	status = "okay";
       };

       &lptimer3 {
       	status = "okay";

       	counter {
       		status = "okay";
       	};
       };

       &ltdc {
       	pinctrl-names = "default";
       	pinctrl-0 = <&ltdc_pins_a>;
       	status = "okay";
       };

       &oem_enc_key {
       	st,non-secure-otp-provisioning;
       };

       &osc_calibration {
       	csi-calibration {
       		status = "okay";
       	};

       	hsi-calibration {
       		status = "okay";
       	};
       };

       &pka {
       	status = "okay";
       };

       &pwr_irq {
       	pinctrl-names = "default";
       	pinctrl-0 = <&wakeup_pins>;
       	status = "okay";
       };

       &rng {
       	status = "okay";
       	clock-error-detect;
       	st,rng-config = <RNG_CONFIG_NIST_B_ID>;
       };

       &rtc {
       	status = "okay";
       };

       &saes {
       	status = "okay";
       };

       &timers12 {
       	status = "okay";

       	counter {
       		status = "okay";
       	};
       };

       &usart1 {
       	pinctrl-names = "default";
       	pinctrl-0 = <&usart1_pins_a>;
       	uart-has-rtscts;
       	status = "disabled";
       };

       &wakeup_pin_5 {
       	bias-pull-up;
       };

       &tzc400 {
       	memory-region = <&optee_framebuffer>;
       };

       &bsec {
       	board_id: board_id@f0 {
       		reg = <0xf0 0x4>;
       		st,non-secure-otp;
       	};
       };

       &uart4 {
       	pinctrl-names = "default";
       	pinctrl-0 = <&uart4_pins_a>;
       	status = "okay";
       };

5. Remove the tamp section:

       &tamp {
       	status = "okay";
       	st,tamp_passive_nb_sample = <4>;
       	st,tamp_passive_sample_clk_div = <16384>;

       	tamp_passive@2 {
       		pinctrl-0 = <&tamp0_in2_pin_a>;
       		status = "okay";
       	};

       	/* Connect pin8 and pin22 from CN8 */
       	tamp_active@1 {
       		pinctrl-0 = <&tamp0_in3_pin_a &tamp0_out1_pin_a>;
       		status = "disabled";
       	};
       };

   This requires us to disable the `stm32_configure_tamp()` function in
   `core/arch/arm/plat-stm32mp1/main.c`. Comment out the body of the function,
   except for the final return statement.

### How to configure clocks from TF-A

1. Make a copy of of the TF-A DTS file, so that it will persist from build to
   build. Note the location and set it in the Builkroot configuration. For
   example, set
   `$(BR2_EXTERNAL_THINKSRS_PATH)/board/stm32mp1/TFA_stm32mp135f-dk.dts` in the
   configuration option `BR2_TARGET_ARM_TRUSTED_FIRMWARE_CUSTOM_DTS_PATH`.

   Likewise, make a copy of the file `stm32mp135f-dk-fw-config.dts`, and include
   it in the same configuration option.

2. Copy the entire `&rcc { ... }` section from the OP-TEE DTS to the TF-A DTS,
   except for the two `pinctrl` lines at the top. In this case, it looks like
   this:

       &rcc {
       	compatible = "st,stm32mp13-rcc", "syscon", "st,stm32mp13-rcc-mco";

       	st,clksrc = <
       		CLK_MPU_PLL1P
       		CLK_AXI_PLL2P
       		CLK_MLAHBS_PLL3
       		CLK_RTC_LSE
       		CLK_MCO1_HSE
       		CLK_MCO2_DISABLED
       		CLK_CKPER_HSE
       		CLK_ETH1_PLL4P
       		CLK_ETH2_PLL4P
       		CLK_SDMMC1_PLL4P
       		CLK_SDMMC2_PLL4P
       		CLK_STGEN_HSE
       		CLK_USBPHY_HSE
       		CLK_I2C4_HSI
       		CLK_I2C5_HSI
       		CLK_USBO_USBPHY
       		CLK_ADC2_CKPER
       		CLK_I2C12_HSI
       		CLK_UART1_HSI
       		CLK_UART2_HSI
       		CLK_UART35_HSI
       		CLK_UART4_HSI
       		CLK_UART6_HSI
       		CLK_UART78_HSI
       		CLK_SAES_AXI
       		CLK_DCMIPP_PLL2Q
       		CLK_LPTIM3_PCLK3
       		CLK_RNG1_PLL4R
       	>;

       	st,clkdiv = <
       		DIV(DIV_MPU, 1)
       		DIV(DIV_AXI, 0)
       		DIV(DIV_MLAHB, 0)
       		DIV(DIV_APB1, 1)
       		DIV(DIV_APB2, 1)
       		DIV(DIV_APB3, 1)
       		DIV(DIV_APB4, 1)
       		DIV(DIV_APB5, 2)
       		DIV(DIV_APB6, 1)
       		DIV(DIV_RTC, 0)
       		DIV(DIV_MCO1, 0)
       		DIV(DIV_MCO2, 0)
       	>;

       	st,pll_vco {
       		pll1_vco_2000Mhz: pll1-vco-2000Mhz {
       			src = <CLK_PLL12_HSE>;
       			divmn = <1 82>;
       			frac = <0xAAA>;
       		};

       		pll1_vco_1300Mhz: pll1-vco-1300Mhz {
       			src = <CLK_PLL12_HSE>;
       			divmn = <2 80>;
       			frac = <0x800>;
       		};

       		pll2_vco_1066Mhz: pll2-vco-1066Mhz {
       			src = <CLK_PLL12_HSE>;
       			divmn = <2 65>;
       			frac = <0x1400>;
       		};

       		pll3_vco_417Mhz: pll3-vco-417Mhz {
       			src = <CLK_PLL3_HSE>;
       			divmn = <1 33>;
       			frac = <0x1a04>;
       		};

       		pll4_vco_600Mhz: pll4-vco-600Mhz {
       			src = <CLK_PLL4_HSE>;
       			divmn = <1 49>;
       		};
       	};

       	/* VCO = 1300.0 MHz => P = 650 (CPU) */
       	pll1: st,pll@0 {
       		compatible = "st,stm32mp1-pll";
       		reg = <0>;

       		st,pll = <&pll1_cfg1>;

       		pll1_cfg1: pll1_cfg1 {
       			st,pll_vco = <&pll1_vco_1300Mhz>;
       			st,pll_div_pqr = <0 1 1>;
       		};

       		pll1_cfg2: pll1_cfg2 {
       			st,pll_vco = <&pll1_vco_2000Mhz>;
       			st,pll_div_pqr = <0 1 1>;
       		};
       	};

       	/* VCO = 1066.0 MHz => P = 266 (AXI), Q = 266, R = 533 (DDR) */
       	pll2: st,pll@1 {
       		compatible = "st,stm32mp1-pll";
       		reg = <1>;

       		st,pll = <&pll2_cfg1>;

       		pll2_cfg1: pll2_cfg1 {
       			st,pll_vco = <&pll2_vco_1066Mhz>;
       			st,pll_div_pqr = <1 1 0>;
       		};
       	};

       	/* VCO = 417.8 MHz => P = 209, Q = 24, R = 11 */
       	pll3: st,pll@2 {
       		compatible = "st,stm32mp1-pll";
       		reg = <2>;

       		st,pll = <&pll3_cfg1>;

       		pll3_cfg1: pll3_cfg1 {
       			st,pll_vco = <&pll3_vco_417Mhz>;
       			st,pll_div_pqr = <1 16 36>;
       		};
       	};

       	/* VCO = 600.0 MHz => P = 50, Q = 10, R = 50 */
       	pll4: st,pll@3 {
       		compatible = "st,stm32mp1-pll";
       		reg = <3>;
       		st,pll = <&pll4_cfg1>;

       		pll4_cfg1: pll4_cfg1 {
       			st,pll_vco = <&pll4_vco_600Mhz>;
       			st,pll_div_pqr = <11 59 11>;
       		};
       	};

       	st,clk_opp {
       		/* CK_MPU clock config for MP13 */
       		st,ck_mpu {

       			cfg_1 {
       				hz = <650000000>;
       				st,clksrc = <CLK_MPU_PLL1P>;
       				st,pll = <&pll1_cfg1>;
       			};

       			cfg_2 {
       				hz = <1000000000>;
       				st,clksrc = <CLK_MPU_PLL1P>;
       				st,pll = <&pll1_cfg2>;
       			};
       		};
       	};
       };

3. From the OP-TEE DTS `$rcc { ... }` section, remove everything except:

       &rcc {
       	compatible = "st,stm32mp13-rcc", "syscon", "st,stm32mp13-rcc-mco";

       	st,clksrc = <
       		CLK_RNG1_PLL4R
       	>;

       	st,clkdiv = <
       		DIV(DIV_MPU, 1)
       	>;
       };

   This is needed because of a bug in the OP-TEE rcc driver, where it attempts
   to set the dividers and clocks even if none are provided.

Now all the clocks are effectively set by TF-A. The only task that OP-TEE has is
to configure the ETZPC to allow certain non-secure accesses.

### Author

Jakob Kastelic, Stanford Research Systems
