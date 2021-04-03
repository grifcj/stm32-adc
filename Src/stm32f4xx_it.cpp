#include <stdio.h>

#include "common.h"

#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;

extern "C"
{
void NMI_Handler(void) { SystemHalt("NMI"); }
void HardFault_Handler(void) { SystemHalt("Hardfault"); }
void MemManage_Handler(void) { SystemHalt("MemManage"); }
void BusFault_Handler(void) { SystemHalt("Busfault"); }
void UsageFault_Handler(void) { SystemHalt("UsageFault"); }

void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }

void SysTick_Handler(void) { HAL_IncTick(); }

void ADC_IRQHandler(void)
{
   HAL_ADC_IRQHandler(&hadc1);
}

void DMA2_Stream0_IRQHandler(void)
{
   HAL_DMA_IRQHandler(hadc1.DMA_Handle);
}

void DMA2_Stream4_IRQHandler(void)
{
   HAL_DMA_IRQHandler(hadc1.DMA_Handle);
}
}
