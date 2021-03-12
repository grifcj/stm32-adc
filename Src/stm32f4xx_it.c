#include "main.h"
#include "common.h"

#include "stm32f4xx_it.h"

void NMI_Handler(void) { SystemHalt("NMI"); }
void HardFault_Handler(void) { SystemHalt("Hardfault"); }
void MemManage_Handler(void) { SystemHalt("MemManage"); }
void BusFault_Handler(void) { SystemHalt("Busfault"); }
void UsageFault_Handler(void) { SystemHalt("UsageFault"); }

void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }

void SysTick_Handler(void) { HAL_IncTick(); }

// TODO Add ADC interrupts for interrupt mode
