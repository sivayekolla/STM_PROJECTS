#include "stm32l476xx.h"

#define GPIOAEN   (1U << 0)
#define GPIOCEN   (1U << 2)

#define PIN5      (1U << 5)
#define PIN13     (1U << 13)

#define LED_PIN   PIN5
#define BTN_PIN   PIN13

// Minimal SystemInit (needed by startup code)
void SystemInit(void)
{
    // Enable FPU (optional)
    SCB->CPACR |= (0xF << 20);

    // Turn on HSI (16 MHz)
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // Disable PLL
    RCC->CR &= ~RCC_CR_PLLON;

    // Reset clock config
    RCC->CFGR = 0x00000000;

    // Flash settings
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_4WS;
}

int main(void)
{
    // Enable clocks
    RCC->AHB2ENR |= GPIOAEN;
    RCC->AHB2ENR |= GPIOCEN;

    // Configure PA5 (LED) as output
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));

    // Configure PC13 (button) as input
    GPIOC->MODER &= ~(3U << (13 * 2));

    while (1)
    {
        if (!(GPIOC->IDR & BTN_PIN))  // Button pressed
        {
            GPIOA->BSRR = LED_PIN;    // LED ON
        }
        else
        {
            GPIOA->BSRR = (LED_PIN << 16);  // LED OFF
        }
    }
}
