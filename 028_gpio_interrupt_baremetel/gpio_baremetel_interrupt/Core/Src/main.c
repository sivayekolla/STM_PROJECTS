#include "stm32l476xx.h"

#define GPIOAEN   (1U << 0)
#define GPIOCEN   (1U << 2)

#define PIN5      (1U << 5)
#define PIN13     (1U << 13)

#define LED_PIN   PIN5
#define BTN_PIN   PIN13

// --------------------------------------------------------------------
// Minimal SystemInit (needed by startup file)
// --------------------------------------------------------------------
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

// --------------------------------------------------------------------
// GPIO + EXTI + NVIC Configuration
// --------------------------------------------------------------------
void gpio_interrupt_init(void)
{
    // 1. Enable GPIOA and GPIOC clocks
    RCC->AHB2ENR |= (GPIOAEN | GPIOCEN);

    // 2. Configure PA5 (LED) as output
    GPIOA->MODER &= ~(3U << (5 * 2));   // Clear mode bits
    GPIOA->MODER |=  (1U << (5 * 2));   // Set as output
    GPIOA->OTYPER &= ~LED_PIN;          // Push-pull
    GPIOA->OSPEEDR |=  (1U << (5 * 2)); // Medium speed
    GPIOA->PUPDR  &= ~(3U << (5 * 2));  // No pull

    // 3. Configure PC13 (Button) as input
    GPIOC->MODER &= ~(3U << (13 * 2));  // Input mode
    GPIOC->PUPDR |=  (1U << (13 * 2));  // Pull-up (optional)

    // 4. Enable SYSCFG clock (EXTI uses SYSCFG multiplexer)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // 5. Map EXTI13 line to port C
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);  // 0x2 = Port C

    // 6. Configure EXTI13 to trigger on falling edge
    EXTI->IMR1  |=  BTN_PIN;    // Unmask line 13
    EXTI->FTSR1 |=  BTN_PIN;    // Falling edge trigger
    EXTI->RTSR1 &= ~BTN_PIN;    // Disable rising edge

    // 7. Enable interrupt in NVIC
    NVIC_SetPriority(EXTI15_10_IRQn, 1);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// --------------------------------------------------------------------
// EXTI interrupt handler (Button press -> Toggle LED)
// --------------------------------------------------------------------
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & BTN_PIN)
    {
        EXTI->PR1 = BTN_PIN;              // Clear pending bit
        GPIOA->ODR ^= LED_PIN;            // Toggle LED
    }
}

// --------------------------------------------------------------------
// main()
// --------------------------------------------------------------------
int main(void)
{
    gpio_interrupt_init();

    while (1)
    {
        // CPU can sleep or loop; LED toggles in ISR
        __WFI();  // Wait For Interrupt (low power)
    }
}
