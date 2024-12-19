#include <stdio.h> // printf()
#include <ctype.h> // isprint()

#include "stm32mp13xx_hal.h"
#include "stm32mp13xx_hal_etzpc.h"
#include "stm32mp13xx_disco.h"
#include "stm32mp13xx_disco_stpmic1.h"

#include "setup.h"

#define READ_TIMEOUT 15000

// global variables
SD_HandleTypeDef SDHandle;

void clear_ddr(uint32_t *ddr_addr, const int num_words)
{
   for (int i=0; i<num_words; i++) {
      uint32_t *p = ddr_addr + 4*i;
      *p = 0;
   }
}

/**
 * @brief Print from DDR memory in the `hexdump -l 16` format.
 *
 * @param ddr_addr Where in DDR to get the data from.
 * @param num_words How many 32-bit words to print.
 */
void print_ddr(uint32_t *ddr_addr, const int num_words)
{
   for (int i=0; i<num_words/4; i+=4) {
      // print address
      printf("0x%08x : ", ddr_addr + 4*i);

      // print in hex
      for (int j=0; j<4; j++) {
         uint32_t *p = ddr_addr + i + j;
         const int i0 = *p;
         for (int k=0; k<4; k++) {
            const char c = (i0 & (0xff << k*8)) >> k*8;
            printf("%02x ", c);
         }
         printf(" ");
      }

      // print as ASCII
      for (int j=0; j<4; j++) {
         uint32_t *p = ddr_addr + i + j;
         const int i0 = *p;
         for (int k=0; k<4; k++) {
            const char c = (i0 & (0xff << k*8)) >> k*8;
            if (isprint(c))
               printf("%c", c);
            else
               printf(".");
         }
      }

      printf("\r\n");
   }
}


void setup_sd(void)
{
   // unsecure SYSRAM so that SDMMC1 (which we configure as non-secure) can access it
   LL_ETZPC_SetSecureSysRamSize(ETZPC, 0);

   // unsecure SDMMC1
   LL_ETZPC_Set_SDMMC1_PeriphProtection(ETZPC, LL_ETZPC_PERIPH_PROTECTION_READ_WRITE_NONSECURE);

   // initialize SDMMC1
   SDHandle.Instance = SDMMC1;
   HAL_SD_DeInit(&SDHandle);

   // SDMMC IP clock xx MHz, SDCard clock xx MHz
   SDHandle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
   SDHandle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
   SDHandle.Init.BusWide             = SDMMC_BUS_WIDE_4B;
   SDHandle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
   SDHandle.Init.ClockDiv            = SDMMC_NSPEED_CLK_DIV;

   if (HAL_SD_Init(&SDHandle) != HAL_OK) {
      printf("Error in HAL_SD_Init()\r\n");
      Error_Handler();
   }

   while (HAL_SD_GetCardState(&SDHandle) != HAL_SD_CARD_TRANSFER)
      ;
}


/**
 * @brief Load data from SD card to DDR memory.
 *
 * @param ddr_addr Where in DDR to place the copied data.
 * @param sd_offset Offset in blocks (sectors).
 * @param num_blocks How many blocks to copy.
 */
void load_blocking(uint8_t *ddr_addr, const int sd_offset, const int num_blocks)
{
   printf("Loading %5d blocks to ddr_addr=0x%08x from SD offset 0x%08x ...\r\n",
         num_blocks, ddr_addr, sd_offset);

   HAL_StatusTypeDef res = HAL_SD_ReadBlocks(
         &SDHandle,
         ddr_addr,
         sd_offset,
         num_blocks,
         READ_TIMEOUT
   );

   if (res == HAL_TIMEOUT)
      printf("HAL_TIMEOUT in HAL_SD_ReadBlocks()\r\n");
   else if (res != HAL_OK) {
      printf("Error in HAL_SD_ReadBlocks()\r\n");
      Error_Handler();
   }
}


int main(void)
{
   HAL_Init();
   SystemClock_Config();
   BSP_PMIC_Init();
   BSP_PMIC_InitRegulators();
   PeriphCommonClock_Config();
   MX_UART4_Init();
   BSP_LED_Init(LED_RED);
   BSP_LED_Init(LED_BLUE);
   setup_ddr();
   setup_sd();

   for (int i=1; i<=3; i++) {
      printf("Hello World %d\r\n", i);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_14);
      HAL_Delay(1000);
   }

   const uint32_t addr_kernel = 0xC2000000;
   const uint32_t addr_dtb    = 0xC6000000;

   // copy DTB and kernel from SD card to DDR memory
   load_blocking((uint8_t*)addr_dtb,    2048, 121);
   load_blocking((uint8_t*)addr_kernel, 4096, 14855);

   // print out the first block of the kernel and DTB:
   printf("First block of the kernel image:\r\n");
   print_ddr((uint32_t*)addr_kernel, 512);
   printf("\nFirst block of the DTB:\r\n");
   print_ddr((uint32_t*)addr_dtb, 512);
   printf("Jumping to kernel...\r\n");

   // jump to kernel, passsing DTB via register r2
   typedef void (*kernel_entry_t)(uint32_t, uint32_t, uint32_t);
   kernel_entry_t kernel_entry = (kernel_entry_t)addr_kernel;
   kernel_entry(0, 0, addr_dtb);
}

// end file main.c

