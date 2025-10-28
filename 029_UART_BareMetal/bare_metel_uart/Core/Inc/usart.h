#ifndef USART_H
#define USART_H

#include "stm32l476xx.h"
#include <stdint.h>

void USART2_Init(void);
void USART2_WriteChar(char c);
void USART2_WriteString(const char *str);
char USART2_ReadChar(void);

#endif
