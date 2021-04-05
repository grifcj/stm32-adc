#include <string.h>

#include "stm32f4xx_hal.h"

#include "common.h"

#include "shell/shell.h"

extern ADC_HandleTypeDef hadc1;

namespace
{

bool isConversionComplete = false;
uint32_t adcData = 0;

bool TimeElapsed(uint32_t startTick, uint32_t timeoutMs)
{
   uint32_t curTick = HAL_GetTick();
   uint32_t elapsed = (curTick > startTick) ? curTick - startTick : 0;
   return elapsed > timeoutMs;
}

void LockInterrupts()
{
   __set_PRIMASK(1);
}

void UnlockInterrupts()
{
   __set_PRIMASK(0);
}

bool IsConversionComplete()
{
   LockInterrupts();
   bool isComplete = isConversionComplete;
   UnlockInterrupts();
   return isComplete;
}

void ReadAndPrintData()
{
   LockInterrupts();
   uint32_t data = adcData;
   isConversionComplete = false;
   adcData = 0;
   UnlockInterrupts();

   shell_print_line("ADC Data: %d", data);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc)
{
   adcData = HAL_ADC_GetValue(adc);
   isConversionComplete = true;
   shell_print_line("Conversion complete");
}

void ADC_Init(bool isContinuous)
{
   hadc1.Instance = ADC1;
   hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
   hadc1.Init.Resolution = ADC_RESOLUTION_12B;
   hadc1.Init.ScanConvMode = DISABLE;
   hadc1.Init.ContinuousConvMode = (isContinuous) ? ENABLE : DISABLE;
   hadc1.Init.DiscontinuousConvMode = DISABLE;
   hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
   hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
   hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
   hadc1.Init.NbrOfConversion = 1;
   hadc1.Init.DMAContinuousRequests = DISABLE;
   hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
   hadc1.ConvCpltCallback = HAL_ADC_ConvCpltCallback;
   if (HAL_ADC_Init(&hadc1) != HAL_OK)
   {
      SystemHalt("HAL_ADC_Init");
   }

   if (HAL_ADC_RegisterCallback(
            &hadc1,
            HAL_ADC_CONVERSION_COMPLETE_CB_ID,
            HAL_ADC_ConvCpltCallback) != HAL_OK)
   {
      SystemHalt("HAL_ADC_RegisterCallback");
   }

   // Configure for the selected ADC regular channel its corresponding rank in
   // the sequencer and its sample time.
   ADC_ChannelConfTypeDef channelConfig = {0};
   channelConfig.Channel = ADC_CHANNEL_0;
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

void StartConversion()
{
   // Start conversion
   if (HAL_ADC_Start_IT(&hadc1) != HAL_OK)
   {
      Log("HAL_ADC_Start_IT failed");
   }

   Log("HAL_ADC_Start_IT");
}

void WaitForConversion()
{
   // Wait until conversion done
   uint32_t startTick = HAL_GetTick();
   const uint32_t TIMEOUT_MS = 1000;
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

      // HAL_Delay(10);
   }
}

void DoSingleConversion()
{
   Log("Do Single Conversion");

   ADC_Init(false);

   while (1)
   {
      uint32_t delayMs = 100;
      HAL_Delay(delayMs);

      StartConversion();
      WaitForConversion();
   }
}

void DoContinuousConversion()
{
   Log("Do Continuous Conversion");

   ADC_Init(true);

   StartConversion();

   while (1)
   {
      WaitForConversion();
   }
}

}

int InterruptConversion(int argc, char* argv[])
{
   bool isContinuous = nullptr != strstr(argv[0], "continuous");
   if (isContinuous)
   {
      DoContinuousConversion();
   }
   else
   {
      DoSingleConversion();
   }

   return 0;
}
