#include <array>
#include <utility>

#include <string.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"

#include "common.h"

#include "shell/shell.h"

extern ADC_HandleTypeDef hadc1;

namespace
{

using T = std::pair<int, int>;
const std::array<T, 3> channelSequence =
{
   std::make_pair(1, ADC_CHANNEL_0),
   std::make_pair(2, ADC_CHANNEL_1),
   std::make_pair(3, ADC_CHANNEL_2)
};
constexpr auto NUM_CHANNELS = std::size(channelSequence);

bool isConversionComplete = false;
uint32_t adcData[NUM_CHANNELS] = {};

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
   uint32_t data[NUM_CHANNELS];

   LockInterrupts();
   isConversionComplete = false;
   memcpy(data, adcData, sizeof(adcData));
   memset(adcData, 0, sizeof(adcData));
   UnlockInterrupts();

   shell_print_line("Received All Channels: %d %d %d",
         data[0], data[1], data[2]);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc)
{
   isConversionComplete = true;
   shell_print_line("Conversion complete");
}

void ADC_Init(void)
{
   hadc1.Instance = ADC1;
   hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
   hadc1.Init.Resolution = ADC_RESOLUTION_12B;
   hadc1.Init.ScanConvMode = ENABLE;
   hadc1.Init.ContinuousConvMode = DISABLE;
   hadc1.Init.DiscontinuousConvMode = DISABLE;
   hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
   hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
   hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
   hadc1.Init.NbrOfConversion = 3;
   hadc1.Init.DMAContinuousRequests = ENABLE;
   hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
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
   for (auto [rank, channel] : channelSequence)
   {
      // the sequencer and its sample time.
      ADC_ChannelConfTypeDef channelConfig = {0};
      channelConfig.Channel = channel;
      channelConfig.Rank = rank;
      channelConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
      if (HAL_ADC_ConfigChannel(&hadc1, &channelConfig) != HAL_OK)
      {
         SystemHalt("HAL_ADC_ConfigChannel");
      }
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
   HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

void SampleAllChannels()
{
   // Start conversion
   if (HAL_ADC_Start_DMA(&hadc1, adcData, sizeof(adcData)) != HAL_OK)
   {
      Log("HAL_ADC_Start_DMA failed");
   }

   Log("HAL_ADC_Start_DMA");

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

      HAL_Delay(10);
   }
}

}

int DmaScanRegular(int argc, char* argv[])
{
   ADC_Init();

   while (1)
   {
      uint32_t delayMs = 100;
      HAL_Delay(delayMs);

      SampleAllChannels();
   }
}
