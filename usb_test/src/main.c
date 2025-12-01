#include "main.h"

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_usart1_tx;

void SystemClock_Config(void)
{
#if !defined(USE_DDR)
  HAL_RCC_DeInit();
    RCC_ClkInitTypeDef RCC_ClkInitStructure;
    RCC_OscInitTypeDef RCC_OscInitStructure;

    /* Enable all available oscillators*/
    RCC_OscInitStructure.OscillatorType = (RCC_OSCILLATORTYPE_HSI |
                                           RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_CSI |
                                           RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE);

    RCC_OscInitStructure.HSIState = RCC_HSI_ON;
    RCC_OscInitStructure.HSEState = RCC_HSE_ON;
    RCC_OscInitStructure.LSEState = RCC_LSE_ON;
    RCC_OscInitStructure.LSIState = RCC_LSI_ON;
    RCC_OscInitStructure.CSIState = RCC_CSI_ON;

    RCC_OscInitStructure.HSICalibrationValue = 0x00; //Default reset value
    RCC_OscInitStructure.CSICalibrationValue = 0x10; //Default reset value
    RCC_OscInitStructure.HSIDivValue = RCC_HSI_DIV1; //Default value

    /* PLL configuration */
    RCC_OscInitStructure.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStructure.PLL.PLLSource = RCC_PLL12SOURCE_HSE;
    RCC_OscInitStructure.PLL.PLLM = 3;
    RCC_OscInitStructure.PLL.PLLN = 81;
    RCC_OscInitStructure.PLL.PLLP = 1;
    RCC_OscInitStructure.PLL.PLLQ = 2;
    RCC_OscInitStructure.PLL.PLLR = 2;
    RCC_OscInitStructure.PLL.PLLFRACV = 0x800;
    RCC_OscInitStructure.PLL.PLLMODE = RCC_PLL_FRACTIONAL;

    RCC_OscInitStructure.PLL2.PLLState = RCC_PLL_ON;
    RCC_OscInitStructure.PLL2.PLLSource = RCC_PLL12SOURCE_HSE;
    RCC_OscInitStructure.PLL2.PLLM = 3;
    RCC_OscInitStructure.PLL2.PLLN = 66;
    RCC_OscInitStructure.PLL2.PLLP = 2;
    RCC_OscInitStructure.PLL2.PLLQ = 2;
    RCC_OscInitStructure.PLL2.PLLR = 1;
    RCC_OscInitStructure.PLL2.PLLFRACV = 0x1400;
    RCC_OscInitStructure.PLL2.PLLMODE = RCC_PLL_FRACTIONAL;

    RCC_OscInitStructure.PLL3.PLLState = RCC_PLL_ON;
    RCC_OscInitStructure.PLL3.PLLSource = RCC_PLL3SOURCE_HSE;
    RCC_OscInitStructure.PLL3.PLLM = 2;
    RCC_OscInitStructure.PLL3.PLLN = 34;
    RCC_OscInitStructure.PLL3.PLLP = 2;
    RCC_OscInitStructure.PLL3.PLLQ = 17;
    RCC_OscInitStructure.PLL3.PLLR = 2;
    RCC_OscInitStructure.PLL3.PLLRGE = RCC_PLL3IFRANGE_1;
    RCC_OscInitStructure.PLL3.PLLFRACV = 0x1a04;
    RCC_OscInitStructure.PLL3.PLLMODE = RCC_PLL_FRACTIONAL;

    RCC_OscInitStructure.PLL4.PLLState = RCC_PLL_ON;
    RCC_OscInitStructure.PLL4.PLLSource = RCC_PLL4SOURCE_HSE;
    RCC_OscInitStructure.PLL4.PLLM = 2;
    RCC_OscInitStructure.PLL4.PLLN = 50;
    RCC_OscInitStructure.PLL4.PLLP = 12;
    RCC_OscInitStructure.PLL4.PLLQ = 60;
    RCC_OscInitStructure.PLL4.PLLR = 6;
    RCC_OscInitStructure.PLL4.PLLRGE = RCC_PLL4IFRANGE_1;
    RCC_OscInitStructure.PLL4.PLLFRACV = 0;
    RCC_OscInitStructure.PLL4.PLLMODE = RCC_PLL_INTEGER;

    /* Enable access to RTC and backup registers */
    SET_BIT(PWR->CR1, PWR_CR1_DBP);
    /* Configure LSEDRIVE value */
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

    if (HAL_RCC_OscConfig(&RCC_OscInitStructure) != HAL_OK) {
      /* HAL RCC configuration error */
      Error_Handler();
      }

    /* Select PLLx as MPU, AXI and MCU clock sources */
    RCC_ClkInitStructure.ClockType = (RCC_CLOCKTYPE_MPU   | RCC_CLOCKTYPE_ACLK  |
                                      RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_PCLK4 |
                                      RCC_CLOCKTYPE_PCLK5 | RCC_CLOCKTYPE_PCLK1 |
                                      RCC_CLOCKTYPE_PCLK6 |
                                      RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3);

    RCC_ClkInitStructure.MPUInit.MPU_Clock = RCC_MPUSOURCE_PLL1;
    RCC_ClkInitStructure.MPUInit.MPU_Div = RCC_MPU_DIV2;
    RCC_ClkInitStructure.AXISSInit.AXI_Clock = RCC_AXISSOURCE_PLL2;
    RCC_ClkInitStructure.AXISSInit.AXI_Div = RCC_AXI_DIV1;
    RCC_ClkInitStructure.MLAHBInit.MLAHB_Clock = RCC_MLAHBSSOURCE_PLL3;
    RCC_ClkInitStructure.MLAHBInit.MLAHB_Div = RCC_MLAHB_DIV1;
    RCC_ClkInitStructure.APB1_Div = RCC_APB1_DIV2;
    RCC_ClkInitStructure.APB2_Div = RCC_APB2_DIV2;
    RCC_ClkInitStructure.APB3_Div = RCC_APB3_DIV2;
    RCC_ClkInitStructure.APB4_Div = RCC_APB4_DIV2;
    RCC_ClkInitStructure.APB5_Div = RCC_APB5_DIV4;
    RCC_ClkInitStructure.APB6_Div = RCC_APB6_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStructure) != HAL_OK) {
      /* HAL RCC configuration error */
      Error_Handler();
      }
#endif
}

static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* Unsecure SYSRAM so that DMA1 (which is always non-secure) can access it. */
  LL_ETZPC_SetSecureSysRamSize(ETZPC, 0);

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  IRQ_SetPriority(DMA1_Stream0_IRQn, 6);
  IRQ_Enable(DMA1_Stream0_IRQn);

}


/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (4000000/10000) - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
void MX_UART4_UART_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
void Error_Handler(void)
{
  BSP_LED_On(LED_BLUE);
  while (1)
  {
  }
}

void assert_failed(uint8_t *file, uint32_t line)
{
  while (1)
  {}
}

void PeriphCommonClock_Config(void)
{
   RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

   /** Initializes the common periph clock
   */
   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
   PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }
   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
   PeriphClkInit.CkperClockSelection = RCC_CKPERCLKSOURCE_HSE;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ETH1;
   PeriphClkInit.Eth1ClockSelection = RCC_ETH1CLKSOURCE_PLL4;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ETH2;
   PeriphClkInit.Eth2ClockSelection = RCC_ETH2CLKSOURCE_PLL4;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SDMMC1;
   PeriphClkInit.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_PLL4;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SDMMC2;
   PeriphClkInit.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_PLL4;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_STGEN;
   PeriphClkInit.StgenClockSelection = RCC_STGENCLKSOURCE_HSE;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
   PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_HSI;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC2;
   PeriphClkInit.Adc2ClockSelection = RCC_ADC2CLKSOURCE_PER;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C12;
   PeriphClkInit.I2c12ClockSelection = RCC_I2C12CLKSOURCE_HSI;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
   PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART4;
   PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_HSI;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SAES;
   PeriphClkInit.SaesClockSelection = RCC_SAESCLKSOURCE_ACLK;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM3;
   PeriphClkInit.Lptim3ClockSelection = RCC_LPTIM3CLKSOURCE_PCLK3;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }
}

static void mx_uart4_init(void)
{
   huart4.Instance = UART4;
   huart4.Init.BaudRate = 115200;
   huart4.Init.WordLength = UART_WORDLENGTH_8B;
   huart4.Init.StopBits = UART_STOPBITS_1;
   huart4.Init.Parity = UART_PARITY_NONE;
   huart4.Init.Mode = UART_MODE_TX_RX;
   huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   huart4.Init.OverSampling = UART_OVERSAMPLING_8;
   huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
   huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
   huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

   if (HAL_UART_Init(&huart4) != HAL_OK)
   {
      Error_Handler();
   }
   if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
   {
      Error_Handler();
   }
   if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
   {
      Error_Handler();
   }
   if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
   {
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
  mx_uart4_init();

  int i = 0;
  while(1) {
     printf("Hello World %d\r\n", i++);
     HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_13);
     HAL_Delay(1000);
  }

//  MX_GPIO_Init();
//  MX_DMA_Init();
//  MX_USB_Device_Init();
//  BSP_LED_Init(LED_BLUE);
//
//  while (1)
//  {
//  }
}
