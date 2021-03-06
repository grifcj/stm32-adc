cmake_minimum_required(VERSION 3.13)
project(ADC LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(
   $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:$<$<VERSION_EQUAL:${CMAKE_CXX_STANDARD},20>:-Wno-volatile>>)

add_library(cmsis INTERFACE)
target_include_directories(cmsis INTERFACE
   Drivers/CMSIS/Core/Include
   Drivers/CMSIS/Device/ST/STM32F4xx/Include)

add_library(hal
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi_ex.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c
   Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c
   )
target_compile_definitions(hal PUBLIC
   STM32F407xx)
target_include_directories(hal PUBLIC
   Drivers/STM32F4xx_HAL_Driver/Inc
   Src)
target_link_libraries(hal PUBLIC cmsis)

add_subdirectory(shell)

file(GLOB srcs CONFIGURE_DEPENDS Src/*.c Src/*.cpp Src/*.s)

add_executable(ADC ${srcs})
target_link_libraries(ADC
   PUBLIC
      g_nano
      hal
      shell)
target_link_options(ADC
   PUBLIC
      -T ${CMAKE_CURRENT_LIST_DIR}/STM32F407IGHx_FLASH.ld
      -Wl,-Map=ADC.map)

