#include <array>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "shell/shell.h"

#include "common.h"

ADC_HandleTypeDef hadc1 = {};

int console_putc(char c);
char console_getc(void);

int PollingSingleConversion(int argc, char* argv[]);
int PollingScanRegular(int argc, char* argv[]);
int InterruptConversion(int argc, char* argv[]);
int DmaScanRegular(int argc, char* argv[]);

static const sShellCommand s_shell_commands[] =
{
   {"polling-single-conversion", PollingSingleConversion, ""},
   {"polling-scan-regular", PollingScanRegular, ""},
   {"interrupt-single-conversion", InterruptConversion, ""},
   {"interrupt-continuous-conversion", InterruptConversion, ""},
   {"dma-scan-regular", DmaScanRegular, ""},
   {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = std::size(s_shell_commands);

void InitHardware()
{
   // Init periperhal driver stack
   HAL_Init();

   // We don't necessarily need clocks for renode tests, but
   // if we were to ever put on hardware would need them
   SystemClock_Config();

   // Use the uart for logging
   UART_Init();
}

void InitShell()
{
   sShellImpl shell_impl =
   {
      .send_char = console_putc,
   };
   shell_boot(&shell_impl);
}

void RunShell()
{
   char c;
   while (true)
   {
      c = console_getc();
      shell_receive_char(c);
   }
}

int main()
{
   InitHardware();
   InitShell();
   RunShell();

   return 0;
}
