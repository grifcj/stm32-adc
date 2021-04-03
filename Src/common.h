#pragma once

#include <stdio.h>

#include "shell/shell.h"

template <typename ...Args>
void shell_print_line(const char *format, Args... args)
{
   static char buf[256] = {};
   snprintf(buf, sizeof(buf), format, args...);
   shell_put_line(buf);
}

void SystemClock_Config(void);
void UART_Init(void);

void SystemHalt(const char* error);
void Log(const char* log);

int console_putc(char c);
char console_getc(void);
