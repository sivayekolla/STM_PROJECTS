#include "stm32l476xx.h"
#include <stdio.h>

void SysTick_Init(void)
{
    SysTick->LOAD = 4000 - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = 5;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = SysTick->VAL;
    uint32_t ticks = ms * 4000;
    while ((start - SysTick->VAL) < ticks);
}

void UART2_Init(void)
{
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    GPIOA->MODER  &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER  |=  ((2 << (2*2)) | (2 << (3*2)));
    GPIOA->AFR[0] &= ~((0xF << (2*4)) | (0xF << (3*4)));
    GPIOA->AFR[0] |=  ((7 << (2*4)) | (7 << (3*4)));
    USART2->BRR = 4000000 / 115200;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void UART2_SendChar(char c)
{
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void UART2_SendString(const char *s)
{
    while (*s)
        UART2_SendChar(*s++);
}

void I2C1_Init(void)
{
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    GPIOB->MODER  &= ~((3 << (6*2)) | (3 << (7*2)));
    GPIOB->MODER  |=  ((2 << (6*2)) | (2 << (7*2)));
    GPIOB->AFR[0] &= ~((0xF << (6*4)) | (0xF << (7*4)));
    GPIOB->AFR[0] |=  ((4 << (6*4)) | (4 << (7*4)));
    GPIOB->OTYPER |= (1 << 6) | (1 << 7);
    GPIOB->OSPEEDR |= (3 << (6*2)) | (3 << (7*2));
    GPIOB->PUPDR &= ~((3 << (6*2)) | (3 << (7*2)));
    GPIOB->PUPDR |=  ((1 << (6*2)) | (1 << (7*2)));
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x00201D2B;
    I2C1->CR1 |= I2C_CR1_PE;
}

uint8_t I2C1_CheckDevice(uint8_t addr)
{
    I2C1->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;
    I2C1->CR2 = (addr << 1) | I2C_CR2_START | (1 << 16);
    while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF | I2C_ISR_STOPF)));
    if (I2C1->ISR & I2C_ISR_NACKF)
    {
        I2C1->ICR |= I2C_ICR_NACKCF;
        I2C1->CR2 |= I2C_CR2_STOP;
        while (!(I2C1->ISR & I2C_ISR_STOPF));
        I2C1->ICR |= I2C_ICR_STOPCF;
        return 1;
    }
    I2C1->TXDR = 0x00;
    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;
    return 0;
}

void SystemClock_HSI4MHz(void)
{
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
}

int main(void)
{
    SystemClock_HSI4MHz();
    SysTick_Init();
    UART2_Init();
    I2C1_Init();
    UART2_SendString("\r\nScanning I2C bus...\r\n");
    char msg[40];
    uint8_t found = 0;
    for (uint8_t addr = 2; addr < 0x7E; addr++)
    {
        if (I2C1_CheckDevice(addr) == 0)
        {
            sprintf(msg, "Found I2C device at 0x%02X\r\n", addr);
            UART2_SendString(msg);
            found = 1;
            delay_ms(5);
        }
    }
    if (!found)
        UART2_SendString("No I2C devices found!\r\n");
    UART2_SendString("Scan complete.\r\n");
    while (1)
        delay_ms(1000);
}
