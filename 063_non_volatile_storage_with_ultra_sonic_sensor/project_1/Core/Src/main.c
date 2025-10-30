#include "stm32l476xx.h"

#define TRIG_PIN     15
#define TRIG_PORT    GPIOB

void delay_us(uint32_t us);
void UART2_Init(void);
void UART2_SendString(char *s);
void TIM1_InputCapture_Init(void);
uint32_t Measure_Distance(void);

void delay_us(uint32_t us)
{
    SysTick->LOAD = 80 * us;          // 80 MHz → 80 ticks/us
    SysTick->VAL = 0;
    SysTick->CTRL = 5;                // Enable, no interrupt
    while (!(SysTick->CTRL & (1 << 16)));
    SysTick->CTRL = 0;
}

void UART2_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

    // PA2 = TX (AF7)
    GPIOA->MODER &= ~(3 << (2 * 2));
    GPIOA->MODER |=  (2 << (2 * 2));
    GPIOA->AFR[0] |= (7 << (2 * 4));

    USART2->BRR = 80000000 / 115200;
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

void TIM1_InputCapture_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // PA8 = TIM1_CH1 (AF1)
    GPIOA->MODER &= ~(3 << (8 * 2));
    GPIOA->MODER |=  (2 << (8 * 2));
    GPIOA->AFR[1]  |= (1 << ((8 - 8) * 4));

    TIM1->PSC = 80 - 1;   // 1 MHz = 1 µs tick
    TIM1->ARR = 0xFFFF;
    TIM1->CCMR1 = 1;      // CC1 as input (IC1 mapped to TI1)
    TIM1->CCER  = 0;      // rising first
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->CR1  |= TIM_CR1_CEN;
}

uint32_t Measure_Distance(void)
{
    uint32_t t1, t2;
    uint32_t timeout;

    // Trigger 10 µs pulse
    GPIOB->ODR |= (1 << TRIG_PIN);
    delay_us(10);
    GPIOB->ODR &= ~(1 << TRIG_PIN);

    // Wait for rising edge
    timeout = 30000;
    while (!(TIM1->SR & TIM_SR_CC1IF))
        if (--timeout == 0) return 0;
    TIM1->SR &= ~TIM_SR_CC1IF;
    t1 = TIM1->CCR1;

    // Change to falling edge
    TIM1->CCER |= TIM_CCER_CC1P;

    // Wait for falling edge
    timeout = 60000;
    while (!(TIM1->SR & TIM_SR_CC1IF))
        if (--timeout == 0) return 0;
    TIM1->SR &= ~TIM_SR_CC1IF;
    t2 = TIM1->CCR1;

    // Reset polarity
    TIM1->CCER &= ~TIM_CCER_CC1P;

    uint32_t diff = (t2 >= t1) ? (t2 - t1) : (0xFFFF - t1 + t2);
    return diff / 58;     // convert µs → cm
}

int main(void)
{
    // Enable clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // TRIG = PB15 output
    GPIOB->MODER &= ~(3 << (TRIG_PIN * 2));
    GPIOB->MODER |=  (1 << (TRIG_PIN * 2));

    UART2_Init();
    TIM1_InputCapture_Init();

    UART2_SendString("Ultrasonic Bare-Metal Test\r\n");

    char msg[50];
    while (1)
    {
        uint32_t dist = Measure_Distance();
        sprintf(msg, "Distance = %lu cm\r\n", dist);
        UART2_SendString(msg);

        for (volatile int i = 0; i < 200000; i++);  // ~200 ms delay
    }
}
