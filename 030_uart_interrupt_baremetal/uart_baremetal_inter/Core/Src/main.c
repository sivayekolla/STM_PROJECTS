#include "stm32l476xx.h"

void UART2_Init(void);
void GPIO_Button_Init(void);
void EXTI15_10_Init(void);
void UART2_SendString(char *str);

/* -------------------- MAIN -------------------- */
int main(void)
{
    SystemInit();                 // default system setup
    UART2_Init();
    GPIO_Button_Init();
    EXTI15_10_Init();

    UART2_SendString("System Ready\r\n");

    while (1)
    {
        __WFI();  // wait for interrupt
    }
}

/* -------------------- UART2 Initialization -------------------- */
void UART2_Init(void)
{
    /* Enable GPIOA and USART2 clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* Configure PA2 (TX) as AF7 (USART2_TX) */
    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->MODER |=  (2U << (2 * 2));
    GPIOA->AFR[0]  &= ~(0xFU << (4 * 2));
    GPIOA->AFR[0]  |=  (7U  << (4 * 2));
    GPIOA->OSPEEDR |=  (3U << (2 * 2));
    GPIOA->OTYPER  &= ~(1U << 2);
    GPIOA->PUPDR   &= ~(3U << (2 * 2));

    /* USART2 config: 115200 baud, 8 N 1 */
    /* For default 4 MHz HSI (no PLL setup) */
    USART2->BRR = 4000000 / 115200;   // ≈ 35 (0x23)
    USART2->CR1 = USART_CR1_TE;       // enable TX
    USART2->CR1 |= USART_CR1_UE;      // enable USART
}

/* -------------------- UART2 Send String -------------------- */
void UART2_SendString(char *str)
{
    while (*str)
    {
        while (!(USART2->ISR & USART_ISR_TXE)); // wait until empty
        USART2->TDR = *str++;
    }
    while (!(USART2->ISR & USART_ISR_TC));      // wait until done
}

/* -------------------- Button GPIO Init (PC13) -------------------- */
void GPIO_Button_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    /* PC13 input mode, no pull-up/pull-down */
    GPIOC->MODER &= ~(3U << (13 * 2));
    GPIOC->PUPDR &= ~(3U << (13 * 2));
}

/* -------------------- EXTI Configuration -------------------- */
void EXTI15_10_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Connect EXTI13 to PC13 */
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);   // Port C = 0b0010

    /* Enable interrupt mask + falling edge trigger */
    EXTI->IMR1  |= (1U << 13);
    EXTI->FTSR1 |= (1U << 13);
    EXTI->RTSR1 &= ~(1U << 13);

    /* Enable EXTI15_10 interrupt in NVIC */
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 1);
}

/* -------------------- Interrupt Handler -------------------- */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1U << 13))
    {
        EXTI->PR1 = (1U << 13);          // clear pending flag
        UART2_SendString("Button Pressed!\r\n");
    }
}
