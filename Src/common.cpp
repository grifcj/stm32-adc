#include "common.h"

#include <stdarg.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"

static UART_HandleTypeDef huart1;

//
// System Clock Configuration
// The system Clock is configured as follow :
//    System Clock source            = PLL (HSE)
//    SYSCLK(Hz)                     = 168000000
//    HCLK(Hz)                       = 168000000
//    AHB Prescaler                  = 1
//    APB1 Prescaler                 = 4
//    APB2 Prescaler                 = 2
//    HSE Frequency(Hz)              = 25000000
//    PLL_M                          = 25
//    PLL_N                          = 336
//    PLL_P                          = 2
//    PLL_Q                          = 7
//    VDD(V)                         = 3.3
//    Main regulator output voltage  = Scale1 mode
//    Flash Latency(WS)              = 5
//
// @note    Renode doesn't care about this. But if we ever try to run
//          on hardware, perhaps it useful.
//
void SystemClock_Config(void)
{
   RCC_ClkInitTypeDef RCC_ClkInitStruct;
   RCC_OscInitTypeDef RCC_OscInitStruct;

   /* Enable Power Control clock */
   __HAL_RCC_PWR_CLK_ENABLE();

   /* The voltage scaling allows optimizing the power consumption when the device is
      clocked below the maximum system frequency, to update the voltage scaling value
      regarding system frequency refer to product datasheet.  */
   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

   /* Enable HSE Oscillator and activate PLL with HSE as source */
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState = RCC_HSE_ON;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLM = 25;
   RCC_OscInitStruct.PLL.PLLN = 336;
   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
   RCC_OscInitStruct.PLL.PLLQ = 7;
   HAL_RCC_OscConfig(&RCC_OscInitStruct);

   /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
      clocks dividers */
   RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
   HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

   /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
   if (HAL_GetREVID() == 0x1001)
   {
      /* Enable the Flash prefetch */
      __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
   }
}

// Initialize UART so that it can be used in simple blocking mode
// transition for implementing stdio output.
void UART_Init(void)
{
   huart1.Instance = USART1;
   huart1.Init.BaudRate = 921600;
   huart1.Init.WordLength = UART_WORDLENGTH_8B;
   huart1.Init.StopBits = UART_STOPBITS_1;
   huart1.Init.Parity = UART_PARITY_NONE;
   huart1.Init.Mode = UART_MODE_TX_RX;
   huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   huart1.Init.OverSampling = UART_OVERSAMPLING_16;
   if (HAL_UART_Init(&huart1) != HAL_OK)
   {
      SystemHalt("Error initializing UART");
   }
}

int console_putc(char c)
{
   HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 0xFFFF);
   return 1;
}

char console_getc(void)
{
   char ret;
   while (HAL_OK != HAL_UART_Receive(&huart1, (uint8_t *)&ret, 1, 0xFFFF));
   return ret;
}

void SystemHalt(const char* error)
{
   shell_print_line("SystemHalt: %s", error);
   while (1)
   {
   }
}

void Log(const char* msg)
{
   shell_print_line("%s", msg);
}
