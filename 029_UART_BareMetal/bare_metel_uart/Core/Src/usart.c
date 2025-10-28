#include "usart.h"

void USART2_Init(void)
{
    // Enable GPIOA and USART2 clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // PA2 = TX (AF7), PA3 = RX (AF7)
    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));  // Clear
    GPIOA->MODER |=  ((2 << (2*2)) | (2 << (3*2)));  // AF mode
    GPIOA->AFR[0]  &= ~((0xF << (4*2)) | (0xF << (4*3)));
    GPIOA->AFR[0]  |=  ((7 << (4*2)) | (7 << (4*3)));

    // Configure USART2: 115200 baud, 8N1
    USART2->CR1 = 0;                     // Disable first
    USART2->BRR = 4000000 / 115200;     // Assuming 80 MHz clock
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE; // Enable TX & RX
    USART2->CR1 |= USART_CR1_UE;         // Enable USART
}

void USART2_WriteChar(char c)
{
    while(!(USART2->ISR & USART_ISR_TXE));  // Wait until TX buffer empty
    USART2->TDR = c;
}

void USART2_WriteString(const char *str)
{
    while(*str)
        USART2_WriteChar(*str++);
}

char USART2_ReadChar(void)
{
    while(!(USART2->ISR & USART_ISR_RXNE)); // Wait until data available
    return (char)(USART2->RDR & 0xFF);
}
