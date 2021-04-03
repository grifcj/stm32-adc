#include <array>
#include <utility>

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

void ADC_Init(void)
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
   hadc1.Init.NbrOfConversion = channelSequence.size();
   hadc1.Init.DMAContinuousRequests = DISABLE;
   hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
   if (HAL_ADC_Init(&hadc1) != HAL_OK)
   {
      SystemHalt("HAL_ADC_Init");
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
}

void SampleAllChannels()
{
   for (auto [rank, channel] : channelSequence)
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
         shell_print_line("Channel: %d ADC Data: %d", rank - 1, data);
      }
   }
}

}

int PollingScanRegular(int argc, char* argv[])
{
   ADC_Init();

   while (1)
   {
      uint32_t delayMs = 100;
      HAL_Delay(delayMs);

      SampleAllChannels();
   }
}
