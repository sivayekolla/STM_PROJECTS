#include "stm32l476xx.h"
#include "usart.h"
#include "delay.h"
#include <stdio.h>

int main(void)
{
    //SystemInit();
    USART2_Init();

    USART2_WriteString("USART2\r\n");

    uint32_t count = 0;
    char buf[64];

    while(1)
    {
        // 1. Send second counter
        count++;
        sprintf(buf, "Second: %lu\r\n", count);
        USART2_WriteString(buf);
        delay_ms(1000);

        // 2. If any key pressed → echo back
        if (USART2->ISR & USART_ISR_RXNE)
        {
            char ch = USART2_ReadChar();
            USART2_WriteString("Echo: ");
            USART2_WriteChar(ch);
            USART2_WriteString("\r\n");
        }
    }
}
