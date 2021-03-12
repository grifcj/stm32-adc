#include "main.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

ADC_HandleTypeDef hadc1 = {};

static void ADC_Init(void)
{
   hadc1.Instance = ADC1;
   hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
   hadc1.Init.Resolution = ADC_RESOLUTION_12B;
   hadc1.Init.ScanConvMode = DISABLE;
   hadc1.Init.ContinuousConvMode = DISABLE;
   hadc1.Init.DiscontinuousConvMode = DISABLE;
   hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
   hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
   hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
   hadc1.Init.NbrOfConversion = 1;
   hadc1.Init.DMAContinuousRequests = DISABLE;
   hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
   if (HAL_ADC_Init(&hadc1) != HAL_OK)
   {
      SystemHalt("HAL_ADC_Init");
   }

   // Configure for the selected ADC regular channel its corresponding rank in
   // the sequencer and its sample time.
   ADC_ChannelConfTypeDef channelConfig = {0};
   channelConfig.Channel = ADC_CHANNEL_1;
   channelConfig.Rank = 1;
   channelConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
   if (HAL_ADC_ConfigChannel(&hadc1, &channelConfig) != HAL_OK)
   {
      SystemHalt("HAL_ADC_ConfigChannel");
   }
}

void InitHardware()
{
   // Init periperhal driver stack
   HAL_Init();

   // We don't necessarily need clocks for renode tests, but
   // if we were to ever put on hardware would need them
   SystemClock_Config();

   // Use the uart for logging
   UART_Init();

   // We're testing ADC so initialize it of course
   ADC_Init();

   // Button used for triggering send
   BSP_PB_Init(BUTTON_TAMPER, BUTTON_MODE_GPIO);
}

void SampleOneChannel()
{
   Log("Try adc conversion");

   // Start conversion
   if (HAL_ADC_Start(&hadc1) != HAL_OK)
   {
      Log("HAL_ADC_Start failed");
   }

   // Wait until conversion done
   uint32_t timeoutMs = 100;
   if (HAL_ADC_PollForConversion(&hadc1, timeoutMs) != HAL_OK)
   {
      Log("Timeout waiting for conversion to complete");
   }
   else
   {
      // Get sample
      uint32_t data = HAL_ADC_GetValue(&hadc1);
      printf("Data read: %d\n", data);
   }

   // Stop conversion...although should stop after single conversion
   if (HAL_ADC_Stop(&hadc1) != HAL_OK)
   {
      Log("HAL_ADC_Stop");
   }
}

int main(void)
{
   InitHardware();

   while (1)
   {
      static const int KEY_PRESSED = 0x01;
      static const int KEY_RELEASED = 0x00;

      // User presses tamper button to send message
      if (BSP_PB_GetState(BUTTON_TAMPER) == KEY_PRESSED)
      {
         // Wait until button released before continuing
         while (BSP_PB_GetState(BUTTON_TAMPER) != KEY_RELEASED)
         {
         }

         SampleOneChannel();
      }
   }
}
