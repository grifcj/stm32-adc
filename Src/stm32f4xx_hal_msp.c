#include "stm32f4xx_hal.h"

void HAL_MspInit(void)
{
   __HAL_RCC_SYSCFG_CLK_ENABLE();
   __HAL_RCC_PWR_CLK_ENABLE();

   HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(hadc->Instance==ADC1)
   {
      __HAL_RCC_ADC1_CLK_ENABLE();
      __HAL_RCC_GPIOA_CLK_ENABLE();

      GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 |GPIO_PIN_3;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
   }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
   if(hadc->Instance==ADC1)
   {
      __HAL_RCC_ADC1_CLK_DISABLE();

      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
   }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if (huart->Instance == USART1)
   {
      __HAL_RCC_USART1_CLK_ENABLE();
      __HAL_RCC_GPIOA_CLK_ENABLE();

      // PA9 == USART1_TX
      // PA10 == USART1_RX
      GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
   }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
   if(huart->Instance==USART1)
   {
      __HAL_RCC_USART1_CLK_DISABLE();
      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
   }
}
