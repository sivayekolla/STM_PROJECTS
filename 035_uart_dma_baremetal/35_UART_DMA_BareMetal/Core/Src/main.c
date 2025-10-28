#include "stm32l476xx.h"

void SystemClock_Config(void);
void UART2_Init(void);
void UART2_SendChar(char c);
void UART2_SendString(char *str);

int main(void)
{
    SystemClock_Config();
    UART2_Init();

    while (1)
    {
        UART2_SendString("Hello\r\n");
        for (volatile int i = 0; i < 2000000; i++);  // small delay
    }
}

void SystemClock_Config(void)
{
    // Enable internal HSI = 16 MHz
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // Switch system clock to HSI
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
}

void UART2_Init(void)
{
    // Enable GPIOA and USART2 clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // PA2 → TX (AF7)
    GPIOA->MODER &= ~(3 << (2 * 2));
    GPIOA->MODER |=  (2 << (2 * 2));
    GPIOA->AFR[0]  &= ~(0xF << (4 * 2));
    GPIOA->AFR[0]  |=  (7 << (4 * 2));
    GPIOA->OSPEEDR |=  (3 << (2 * 2));

    // Configure USART2: 115200 baud, 8N1
    USART2->BRR = 16000000 / 115200;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;  // Enable TX + USART
}

void UART2_SendChar(char c)
{
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void UART2_SendString(char *str)
{
    while (*str)
    {
        UART2_SendChar(*str++);
    }
}
