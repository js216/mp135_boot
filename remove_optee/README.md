# Run Linux on STM32MP135 without OP-TEE

In this example, we show how to minimize the tasks performed by OP-TEE with the
goal of removing it altogether. To this end, we:

- Reassign the STPMIC1 from the trusted execution environment (OP-TEE) to the
  "untrusted" Linux kernel, so that the I2C4 can be unsecured and used directly
  from the kernel and userspace

- Configure clocks (RCC) from Arm Trusted Firmware (TF-A)

- Configure ETZPC from TF-A

- Explain boot process from TF-A to OP-TEE to Linux

- Explain how secure monitor calls (SMC) calls work

- Explain secure interrupts (FIQ) in OP-TEE

- Check for additional SMC calls

- (Work in progress!) Disable SMC calls from Linux

Note that removing OP-TEE is NOT supported by official ST resources, where the
secure OS is strongly recommended to be used for power management using the SCMI
interface, clock configuration, ETZPC setup, etc.

### Prerequisites

This example assumes a running Buildroot-based configuration that boots Linux
and OP-TEE using TF-A, without U-Boot (see `tfa_falcon`).

### STPMIC1 Linux kernel driver with STM32MP135

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

Instead of following the above steps, we can also make the changes to the Linux
files by applying the patch file `0003-fix-stpmic1.patch`. The changes to OP-TEE
files still need to be done manually, unless we want to remove OP-TEE entirely,
in which case the changes are not necessary and will be discarded at the end.

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
   `core/arch/arm/plat-stm32mp1/drivers/stm32mp1_syscfg.c`, comment out the body
   of the function `stm32mp_syscfg_set_hslv()`, except for `return
   TEE_SUCCESS;`. This function is not needed so long as we do not intend to use
   the HSLV mode, i.e., OTP18 bit 13 is not set, and VDD is not below 2.5 V.

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

Now all the clocks are effectively set by TF-A.

# How to configure ETZPC from TF-A

To relieve OP-TEE of the ETZPC configuration duty, follow these steps:

1. To include the ETZPC driver in the Arm Trusted Firmware (TF-A) makefile, add
   the following line at the end of `plat/st/common/common.mk`:

       BL2_SOURCES		+=	drivers/st/etzpc/etzpc.c

2. In the STM32MP1 platform files included in TF-A, the ETZPC DECPROT
   assignments are incorrect. Replace all of `STM32MP1_ETZPC_` defines in
   `plat/st/stm32mp1/stm32mp1_def.h` with the values given in Table 85, "DECPROT
   assignment", of the STM32MP13xx Reference manual:

       #define STM32MP1_ETZPC_VREFBUF_ID	0
       #define STM32MP1_ETZPC_LPTIM2_ID	1
       #define STM32MP1_ETZPC_LPTIM3_ID	2
       #define STM32MP1_ETZPC_LTDC_ID		3
       #define STM32MP1_ETZPC_DCMIPP_ID	4
       #define STM32MP1_ETZPC_USBPHYCTRL_ID	5
       #define STM32MP1_ETZPC_DDRCTRLPHY_ID	6
       #define STM32MP1_ETZPC_IWDG1_ID		12
       #define STM32MP1_ETZPC_STGENC_ID	13
       #define STM32MP1_ETZPC_USART1_ID	16
       #define STM32MP1_ETZPC_USART2_ID	17
       #define STM32MP1_ETZPC_SPI4_ID		18
       #define STM32MP1_ETZPC_SPI5_ID		19
       #define STM32MP1_ETZPC_I2C3_ID		20
       #define STM32MP1_ETZPC_I2C4_ID		21
       #define STM32MP1_ETZPC_I2C5_ID		22
       #define STM32MP1_ETZPC_TIM12_ID		23
       #define STM32MP1_ETZPC_TIM13_ID		24
       #define STM32MP1_ETZPC_TIM14_ID		25
       #define STM32MP1_ETZPC_TIM15_ID		26
       #define STM32MP1_ETZPC_TIM16_ID		27
       #define STM32MP1_ETZPC_TIM17_ID		28
       #define STM32MP1_ETZPC_ADC1_ID		32
       #define STM32MP1_ETZPC_ADC2_ID		33
       #define STM32MP1_ETZPC_OTG_ID		34
       #define STM32MP1_ETZPC_RNG_ID		40
       #define STM32MP1_ETZPC_HASH_ID		41
       #define STM32MP1_ETZPC_CRYP_ID		42
       #define STM32MP1_ETZPC_SAES_ID		43
       #define STM32MP1_ETZPC_PKA_ID		44
       #define STM32MP1_ETZPC_BKPSRAM_ID	45
       #define STM32MP1_ETZPC_ETH1_ID		48
       #define STM32MP1_ETZPC_ETH2_ID		49
       #define STM32MP1_ETZPC_SDMMC1_ID	50
       #define STM32MP1_ETZPC_SDMMC2_ID	51
       #define STM32MP1_ETZPC_MCE_ID		53
       #define STM32MP1_ETZPC_FMC_ID		54
       #define STM32MP1_ETZPC_QSPI_ID		55
       #define STM32MP1_ETZPC_SRAM1_ID		60
       #define STM32MP1_ETZPC_SRAM2_ID		61
       #define STM32MP1_ETZPC_SRAM3_ID		62


3. Add the ETZPC node to the TF-A DTS file (`fdts/stm32mp131.dtsi`), under `soc
   { ... }`:

       etzpc: etzpc@5c007000 {
       	compatible = "st,stm32mp13-etzpc";
       	reg = <0x5c007000 0x400>;
       	clocks = <&rcc TZPC>;
       	#address-cells = <1>;
       	#size-cells = <1>;
       };

4. In the platform-specific TF-A BL2 (that is, bootloader) file
   `plat/st/stm32mp1/bl2_plat_setup.c`, add the ETZPC driver header file:

       #include <drivers/st/etzpc.h>

   Later in the same file, at the end of the function `bl2_platform_setup()`,
   add the desired ETZPC configuration:

       /* Configure ETZPC */
       if (etzpc_init() != 0) {
       	panic();
       }
       etzpc_configure_decprot(STM32MP1_ETZPC_SDMMC1_ID, ETZPC_DECPROT_NS_RW);

5. Repeat this process with as many `etzpc_configure_decprot()` calls as
   required for one's needs. For example, to duplicate the OP-TEE configuration
   (except for unsecuring I2C4, as per the steps at the beginning of this
   guide), we need the following:

       etzpc_configure_decprot(STM32MP1_ETZPC_ADC1_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_ADC2_ID,       ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_BKPSRAM_ID,    ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_CRYP_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_DCMIPP_ID,     ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_DDRCTRLPHY_ID, ETZPC_DECPROT_NS_R_S_W);
       etzpc_configure_decprot(STM32MP1_ETZPC_ETH1_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_ETH2_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_FMC_ID,        ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_HASH_ID,       ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_I2C3_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_I2C4_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_I2C5_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_IWDG1_ID,      ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_LPTIM2_ID,     ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_LPTIM3_ID,     ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_LTDC_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_MCE_ID,        ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_OTG_ID,        ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_PKA_ID,        ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_QSPI_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_RNG_ID,        ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SAES_ID,       ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SDMMC1_ID,     ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SDMMC2_ID,     ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SPI4_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SPI5_ID,       ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SRAM1_ID,      ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SRAM2_ID,      ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_SRAM3_ID,      ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_STGENC_ID,     ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_TIM12_ID,      ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_TIM13_ID,      ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_TIM14_ID,      ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_TIM15_ID,      ETZPC_DECPROT_S_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_TIM16_ID,      ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_TIM17_ID,      ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_USART1_ID,     ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_USART2_ID,     ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_USBPHYCTRL_ID, ETZPC_DECPROT_NS_RW);
       etzpc_configure_decprot(STM32MP1_ETZPC_VREFBUF_ID,    ETZPC_DECPROT_NS_RW);

6. Remove the ETZPC section (`&etzpc {...}`) from the OP-TEE DTS file
   `core/arch/arm/dts/stm32mp135f-dk.dts`.

Instead of following steps 1 through 6, we can apply the patch file
`0001-allow-large-bl33-and-config-etzpc-rcc.patch` to the TF-A code.

### Boot process from TF-A to OP-TEE to Linux

When Arm Trusted Firmware (TF-A) is done with its own initialization, it loads
several images into memory. In the STM32MP1 case, these are defined in the
array `bl2_mem_params_desc` in file
`plat/st/stm32mp1/plat_bl2_mem_params_desc.c`, and include the following:

- `FW_CONFIG_ID`: firmware config, which is mostly just the information on
TrustZone memory regions that is used by TF-A itself

- `BL32_IMAGE_ID`: the OP-TEE executable

- `BL32_EXTRA1_IMAGE_ID`, `BL32_EXTRA2_IMAGE_ID`, and `TOS_FW_CONFIG_ID`: some
  stuff needed by OP-TEE

- `BL33_IMAGE_ID`: the non-trusted bootloader (U-Boot) or directly Linux itself,
  if operating in the "falcon mode"

- `HW_CONFIG_ID`: the Device Tree Blob (DTB) used by U-Boot or Linux, whichever
  is run as "BL32"

Just before passing control to OP-TEE, the TF-A prints a couple messages in the
`bl2_main()` function (`bl2/bl2_main.c`), and then runs `bl2_run_next_image`
(`bl2/aarch32/bl2_run_next_image.S`). There, we disable MMU, put the OP-TEE
entry address into the link register (either `lr` or `lr_svc`), load the `SPSR`
register, and then do an "exception return" to atomically change the program
counter to the link register value, and restore the Current Program Status
Register (CPSR) from the Saved Program Status Register (SPSR).

### How do secure monitor calls (SMC) work?

The ARMv7-A architecture provides optional TrustZone extension, which are
implemented on the STM32MP135 chips (as well as the virtualization
extension). In this scheme, the processor is at all times executing in one of
two "worlds", either the secure or the non-secure one.

The `NS` bit of the
[`SCR`](https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Control-Registers-in-a-VMSA-implementation/VMSA-System-control-registers-descriptions--in-register-order/SCR--Secure-Configuration-Register--Security-Extensions?lang=en)
register defines which world we're currently in. If `NS=1`, we are in non-secure
world, otherwise we're in the secure world. The one exception to this is that
when the processor is running in
[Monitor mode](https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/ARM-processor-modes-and-ARM-core-registers/ARM-processor-modes?lang=en#CIHGHDGI);
in that case, the code is executing the secure world and `SCR.NS` merely
indicates which world the processor was in before entering the Monitor mode.
(The current processor mode is given by the `M` bits of the
[`CPSR`](https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/ARM-processor-modes-and-ARM-core-registers/Program-Status-Registers--PSRs-?lang=en#CIHBFGJG)
register.)

The processor starts execution in the secure world. How do we transition to the
non-secure world? Outside of Monitor mode, Arm does not recommend direct
manipulation of the `SCR.NS` bit to change from the secure world to the
non-secure world or vice versa. Instead, the right way is to first change into
Monitor mode, flip the `SCR.NS` bit, and leave monitor mode. To enter Monitor
mode, execute the `SMC` instruction. This triggers the SMC exception, and the
processor begins executing the SMC handler.

The location of the SMC handler has to be previously stored in the
[MVBAR register](https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Control-Registers-in-a-VMSA-implementation/VMSA-System-control-registers-descriptions--in-register-order/MVBAR--Monitor-Vector-Base-Address-Register--Security-Extensions).
The initial setup required is as follows:

1. Write a SMC handler. As an example, consult OP-TEE source code, which
   provides the handler `sm_smc_entry`, defined in `core/arch/arm/sm/sm_a32.S`.

2. Create a vector table for monitor mode. As specified in the
   [Arm architecture](https://developer.arm.com/documentation/ddi0406/b/System-Level-Architecture/The-System-Level-Programmers--Model/Exceptions/Exception-vectors-and-the-exception-base-address?lang=en)
   manual, the monitor vector table has eight entries:

   1. Unused
   2. Unused
   3. Secure Monitor Call (SMC) handler
   4. Prefetch Abort handler
   5. Data Abort handler
   6. Unused
   7. IRQ interrupt handler
   8. FIQ interrupt handler

   Obviously entry number 3 has to point to the SMC handler defined previously.
   For example, OP-TEE defines the following vector table in
   `core/arch/arm/sm/sm_a32.S`:

       LOCAL_FUNC sm_vect_table , :, align=32
       UNWIND(	.cantunwind)
       	b	.		/* Reset			*/
       	b	.		/* Undefined instruction	*/
       	b	sm_smc_entry	/* Secure monitor call		*/
       	b	.		/* Prefetch abort		*/
       	b	.		/* Data abort			*/
       	b	.		/* Reserved			*/
       	b	.		/* IRQ				*/
       	b	sm_fiq_entry	/* FIQ				*/
       END_FUNC sm_vect_table

   We see only the SMC and FIQ handlers are installed, since OP-TEE setup
   disables all other Monitor-mode interrupts and exceptions.

3. Install the vector table to the MVBAR register. The OP-TEE source code
   defines the followign macros in `out/core/include/generated/arm32_sysreg.h`:

       /* Monitor Vector Base Address Register */
       static inline __noprof uint32_t read_mvbar(void)
       {
       	uint32_t v;

       	asm volatile ("mrc p15, 0, %0, c12, c0, 1" : "=r"  (v));

       	return v;
       }

       /* Monitor Vector Base Address Register */
       static inline __noprof void write_mvbar(uint32_t v)
       {
       	asm volatile ("mcr p15, 0, %0, c12, c0, 1" : : "r"  (v));
       }

   This merely follows the Arm manual on how to access the
   [MVBAR register](https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Control-Registers-in-a-VMSA-implementation/VMSA-System-control-registers-descriptions--in-register-order/MVBAR--Monitor-Vector-Base-Address-Register--Security-Extensions).

With this setup in place, to transition from the secure world to the non-secure
world, the steps are as follows:

1. Place the arguments to the SMC handler into registers `r0` through `r4` (or
   as many as are needed by the handler), and execute the SMC instruction. For
   example, just before passing control to the non-secure world, OP-TEE
   `reset_primary` function (called from the `_start` function) does the
   following:

       mov	r4, #0
       mov	r3, r6
       mov	r2, r7
       mov	r1, #0
       mov	r0, #TEESMC_OPTEED_RETURN_ENTRY_DONE
       smc	#0

2. This puts the processor into Monitor mode, and it begins execution at the
   previously-installed SMC handler. The handler stores secure-mode registers
   into some memory location for future use, then sets the `SCR.NS` bit:

       read_scr r0
       orr	r0, r0, #(SCR_NS | SCR_FIQ) /* Set NS and FIQ bit in SCR */
       write_scr r0

   This also sets the `SCR.FIQ` bit, which means that FIQ interrupts are also
   taken to Monitor mode. In this way, OP-TEE assigns IRQ interrupts to the
   non-secure world, and FIQ interrupts to the secure-world. Of course, this
   means that the Monitor-mode vector table needs a FIQ handler (as mentioned
   in passing above), and the system interrupt handler (GIC on STM32MP135) needs
   to be configured to pass "secure" interrupts as FIQ.

3. After adjusting the stack pointer and restoring the nonsecure register values
   from the stack, the SMC handler returns:

       add	sp, sp, #(SM_CTX_NSEC + SM_NSEC_CTX_R0)
       pop	{r0-r7}
       rfefd	sp!

   The return location and processor mode is stored on the stack and
   automatically retrieved by the `rfefd sp!` instruction. Of course this means
   they have to be previously stored in the right place on the stack; see
   `sm_smc_entry` source code for details.

### Secure interrupts in OP-TEE

As mentioned above, OP-TEE code, before returning to non-secure mode, enables
the `SCR.FIQ` bit, which means that FIQ interrupts get taken to Monitor mode,
serviced by the FIQ handler that is installed in the Monitor-mode vector table
(the table address is stored in the `MVBAR` register).

As mentioned above, an arbitrary number of system interrupts may be passed as a
FIQ to the processor core. OP-TEE handles these interrupts in `itr_handle()`
(defined in `core/kernel/interrupt.c`). The individual interrupt handlers are
stored in a linked list, which `itr_handle()` traverses until it finds a handler
whose interrupt number (`h->it`) matches the given interrupt.

For example, the handler for the TZC interrupt (TrustZone memory protection) is
defined in `core/arch/arm/plat-stm32mp1/plat_tzc400.c`, as the
`tzc_it_handler()` function.

### Check for additional SMC calls

At this stage, Linux is still issuing many SMC calls. We can see that by
inserting a function that prints a message into the OP-TEE SMC handler, as
follows.

1. In OP-TEE, define two "print" functions in `core/arch/arm/kernel/boot.c`:

       void __print_from_secure_smc(void)
       {
          EMSG_RAW("SMC from Secure");
       }

       void __print_from_nonsecure_smc(void)
       {
          EMSG_RAW("SMC from Non-Secure");
       }

2. Call these two functions from the OP-TEE secure monitor call (SMC) handler.
   Open `core/arch/arm/sm/sm_a32.S`, in function `sm_smc_entry`, and call
   `__print_from_secure_smc()` after checking the `SCR_NS` bit:

       /* Find out if we're doing an secure or non-secure entry */
       read_scr r1
       tst	r1, #SCR_NS
       bne	.smc_from_nsec
       bl __print_from_secure_smc

   A couple lines later, insert the "print from nonsecure":

       .smc_from_nsec:
       	bl __print_from_nonsecure_smc

### Disable Linux SMC calls (work in progress)

Since clock and power configuration is done outside of OP-TEE, we need to
configure the Linux kernel so that it does not do unnecessary SMC calls into
OP-TEE.

1. Unset the following Linux kernel configuration options:

       CONFIG_ARM_SCMI_POWER_DOMAIN
       CONFIG_SENSORS_ARM_SCMI
       CONFIG_REGULATOR_ARM_SCMI
       CONFIG_RESET_SCMI
       CONFIG_ARM_SCMI_TRANSPORT_MAILBOX
       CONFIG_TRUSTED_FOUNDATIONS
       CONFIG_ARM_SMCC_SOC_ID
       CONFIG_ARM_SMC_WATCHDOG

       CONFIG_ARM_STM32_CPUFREQ
       CONFIG_CPUFREQ_DT
       CONFIG_CPU_FREQ_GOV_SCHEDUTIL
       CONFIG_CPU_FREQ_GOV_CONSERVATIVE
       CONFIG_CPU_FREQ_GOV_USERSPACE
       CONFIG_CPU_FREQ_GOV_POWERSAVE
       CONFIG_CPU_FREQ_STAT
       CONFIG_CPU_FREQ
       CONFIG_CPU_IDLE

2. The following are required or it doesn't boot ...

       CONFIG_COMMON_CLK_SCMI
       CONFIG_RTC_DRV_STM32
       CONFIG_ARM_SCMI_PROTOCOL
       CONFIG_ARM_SCMI_TRANSPORT_OPTEE
       CONFIG_ARM_SCMI_TRANSPORT_SMC
       CONFIG_ARM_SCMI_HAVE_TRANSPORT
       CONFIG_ARM_SCMI_HAVE_SHMEM
       CONFIG_ARM_SCMI_HAVE_MSG
       CONFIG_OPTEE
       CONFIG_TEE

### Author

Jakob Kastelic, Stanford Research Systems
