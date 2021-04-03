#include <array>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shell/shell.h"

extern "C"
{
#include "common.h"
}

int PollingSingleConversion(int argc, char* argv[]);
int PollingScanRegular(int argc, char* argv[]);
int InterruptSingleConversion(int argc, char* argv[]);
int InterruptScanRegular(int argc, char* argv[]);
int DmaSingleConversion(int argc, char* argv[]);
int DmaScanRegular(int argc, char* argv[]);

static const sShellCommand s_shell_commands[] =
{
   {"polling-single-conversion", PollingSingleConversion, ""},
   {"polling-scan-regular", PollingScanRegular, ""},
   {"interrupt-single-conversion", InterruptSingleConversion, ""},
   {"interrupt-scan-regular", InterruptScanRegular, ""},
   {"dma-single-conversion", DmaSingleConversion, ""},
   {"dma-scan-regular", DmaScanRegular, ""},
   {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
constexpr size_t g_num_shell_commands = std::size(s_shell_commands);

ADC_HandleTypeDef hadc1 = {};

static const std::array channelSequence = {
   std::make_pair(1, ADC_CHANNEL_1),
   std::make_pair(2, ADC_CHANNEL_2),
   std::make_pair(3, ADC_CHANNEL_3)};
static constexpr auto NUM_CHANNELS = std::size(channelSequence);

bool isConversionComplete = false;
uint32_t adcData[NUM_CHANNELS] = {};

static void ADC_Init(void)
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
   uint32_t data[NUM_CHANNELS];

   Lock();
   isConversionComplete = false;
   memcpy(data, adcData, sizeof(adcData));
   Unlock();

   printf("Received All Channels: ");
   for (size_t idx = 0; idx < NUM_CHANNELS - 1; ++idx)
   {
      printf("%d ", data[idx]);
   }
   printf("%d\n", data[NUM_CHANNELS - 1]);
}

void SampleOneChannel()
{
   Log("Try interrupt adc conversion");

   // Start conversion
   if (HAL_ADC_Start_DMA(&hadc1, adcData, sizeof(adcData)) != HAL_OK)
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
   // (void)HAL_ADC_GetValue(adc);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc)
{
   static int count = 0;
   printf("ADC Error! count=%d\n", count++);
}

int console_putc(char c) {
  app_uart_put(c);
  return 1;
}

char console_getc(void) {
  uint8_t cr;
  while (app_uart_get(&cr) != NRF_SUCCESS);
  return (char)cr;
}

int main()
{
   InitHardware();

   sShellImpl shell_impl =
   {
      .send_char = console_putc,
   };
   shell_boot(&shell_impl);

   char c;
   while (true)
   {
      c = console_getc();
      shell_receive_char(c);
   }

   while (1)
   {
      uint32_t delayMs = 100;
      HAL_Delay(delayMs);

      SampleOneChannel();
   }
}
