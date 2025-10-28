#include "stm32l476xx.h"

void UART2_Init(void);
void UART2_SendString(char *str);
void delay_ms(uint32_t ms);
void TIM2_Init(void);

int main(void)
{
    SystemInit();
    UART2_Init();
    TIM2_Init();

    while(1)
    {
        UART2_SendString("every 5 sec\r\n");
        delay_ms(5000);
    }
}

void UART2_Init(void)
{
    // Enable GPIOA and USART2 clocks
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // Configure PA2 (TX) and PA3 (RX) as AF7 (USART2)
    GPIOA->MODER   &= ~((3<<4) | (3<<6));     // clear bits
    GPIOA->MODER   |=  (2<<4) | (2<<6);       // alternate mode
    GPIOA->AFR[0]  &= ~((0xF<<8) | (0xF<<12));
    GPIOA->AFR[0]  |=  (7<<8) | (7<<12);      // AF7 for PA2/PA3
    GPIOA->OSPEEDR |=  (3<<4) | (3<<6);       // high speed

    // USART2 configuration: 115200 baud, 8N1 (assuming 80 MHz PCLK1)
    USART2->BRR = 16000000 / 115200;          // = 694
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE; // enable TX + USART
}

void UART2_SendString(char *str)
{
    while(*str)
    {
        while(!(USART2->ISR & USART_ISR_TXE));  // wait for empty
        USART2->TDR = *str++;
    }
    while(!(USART2->ISR & USART_ISR_TC)); // wait for complete
}

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 4000; i++)
        __NOP();
}

void TIM2_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    TIM2->PSC = 8000 - 1;    // 10 kHz
    TIM2->ARR = 10000 - 1;   // 1 s overflow
    TIM2->CR1 |= TIM_CR1_CEN;
}
