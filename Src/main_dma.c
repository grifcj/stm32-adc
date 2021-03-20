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

   // This handle only needed for this ADC
   static DMA_HandleTypeDef hdma;

   __HAL_RCC_DMA2_CLK_ENABLE();

   // Configure ADC stream to transfer samples to memory
   hdma.Instance = DMA2_Stream0;
   hdma.Init.Channel  = DMA_CHANNEL_0;
   hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
   hdma.Init.PeriphInc = DMA_PINC_DISABLE;
   hdma.Init.MemInc = DMA_MINC_ENABLE;
   hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
   hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
   hdma.Init.Mode = DMA_CIRCULAR;
   hdma.Init.Priority = DMA_PRIORITY_HIGH;
   hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
   hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
   hdma.Init.MemBurst = DMA_MBURST_SINGLE;
   hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
   HAL_DMA_Init(&hdma);

   // Associate the initialized DMA handle to the the ADC handle
   __HAL_LINKDMA(&hadc1, DMA_Handle, hdma);

   // Enable interrupt
   /* HAL_NVIC_SetPriority(ADC_IRQn, 4, 0); */
   /* HAL_NVIC_EnableIRQ(ADC_IRQn); */
   /*  */
   HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
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
   if (HAL_ADC_Start_DMA(&hadc1, &adcData, sizeof(adcData)) != HAL_OK)
   {
      Log("HAL_ADC_Start_DMA failed");
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
