#include "uart.h"

void UART2_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER &= ~( (3U<<(2*2)) | (3U<<(3*2)) );
    GPIOA->MODER |=  ( (2U<<(2*2)) | (2U<<(3*2)) );
    GPIOA->AFR[0] |= (7U<<(2*4)) | (7U<<(3*4));

    USART2->BRR = 80000000/115200;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void UART2_SendChar(char c)
{
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void UART2_SendString(char *s)
{
    while (*s) UART2_SendChar(*s++);
}

void UART2_SendHex(uint8_t val)
{
    char hex[3];
    const char* map="0123456789ABCDEF";
    hex[0]=map[(val>>4)&0xF];
    hex[1]=map[val&0xF];
    hex[2]='\0';
    UART2_SendString(hex);
}
