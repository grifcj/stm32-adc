#include "main.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

ADC_HandleTypeDef hadc1 = {};
bool isConversionComplete = false;
uint32_t adcData = 0;

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

   // Enable interrupt
   HAL_NVIC_SetPriority(ADC_IRQn, 4, 0);
   HAL_NVIC_EnableIRQ(ADC_IRQn);
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
}

bool TimeElapsed(uint32_t startTick, uint32_t timeoutMs)
{
   uint32_t curTick = HAL_GetTick();
   uint32_t elapsed = (curTick > startTick) ? curTick - startTick : 0;
   return elapsed > timeoutMs;
}

void Lock()
{
   HAL_NVIC_DisableIRQ(ADC_IRQn);
}

void Unlock()
{
   HAL_NVIC_EnableIRQ(ADC_IRQn);
}

bool IsConversionComplete()
{
   Lock();
   bool isComplete = isConversionComplete;
   Unlock();
   return isComplete;
}

void ReadAndPrintData()
{
   Lock();
   uint32_t data = adcData;
   isConversionComplete = false;
   adcData = 0;
   Unlock();

   printf("ADC Data: %d\n", data);
}

void SampleOneChannel()
{
   Log("Try interrupt adc conversion");

   // Start conversion
   if (HAL_ADC_Start_IT(&hadc1) != HAL_OK)
   {
      Log("HAL_ADC_Start_IT failed");
   }

   // Wait until conversion done
   uint32_t startTick = HAL_GetTick();
   const uint32_t TIMEOUT_MS = 100;
   while (1)
   {
      if (TimeElapsed(startTick, TIMEOUT_MS))
      {
         break;
      }

      if (IsConversionComplete())
      {
         ReadAndPrintData();
         break;
      }

      HAL_Delay(10);
   }
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc)
{
   isConversionComplete = true;
   adcData = HAL_ADC_GetValue(adc);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc)
{
   static int count = 0;
   printf("ADC Error! count=%d\n", count++);
}

int main(void)
{
   InitHardware();

   while (1)
   {
      uint32_t delayMs = 100;
      HAL_Delay(delayMs);

      SampleOneChannel();
   }
}
