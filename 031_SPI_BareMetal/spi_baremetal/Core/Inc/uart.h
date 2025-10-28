#ifndef UART_H
#define UART_H

#include "stm32l476xx.h"
#include <stdint.h>

void UART2_Init(void);
void UART2_SendChar(char c);
void UART2_SendString(char *s);
void UART2_SendHex(uint8_t val);

#endif
