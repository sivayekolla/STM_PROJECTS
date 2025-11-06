#include "stm32l476xx.h"
#include <stdio.h>

#define TRIG_PIN 15
#define TRIG_PORT GPIOB

// ------------------------ Delay ------------------------
void delay_us(uint32_t us) {
    for(uint32_t i = 0; i < us * 10; i++) __NOP(); // crude ~1us
}

void delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms * 8000; i++) __NOP(); // crude ~1ms
}

// ------------------------ UART ------------------------
void uart_init(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;  // enable USART2
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;     // enable GPIOA

    // PA2=TX, PA3=RX -> AF7
    GPIOA->MODER &= ~((3<<4)|(3<<6));
    GPIOA->MODER |= ((2<<4)|(2<<6));         // AF mode
    GPIOA->AFR[0] &= ~((0xF<<8)|(0xF<<12));
    GPIOA->AFR[0] |= ((7<<8)|(7<<12));       // AF7

    USART2->BRR = 4000000/115200;            // assuming PCLK1=4MHz
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_send_char(char c) {
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void uart_send_str(char *str) {
    while(*str) uart_send_char(*str++);
}

// ------------------------ GPIO ------------------------
void gpio_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;  // enable GPIOB
    // PB15 output (TRIG)
    TRIG_PORT->MODER &= ~(3 << (TRIG_PIN*2));
    TRIG_PORT->MODER |=  (1 << (TRIG_PIN*2));
    TRIG_PORT->OTYPER &= ~(1<<TRIG_PIN);
    TRIG_PORT->OSPEEDR |= (3 << (TRIG_PIN*2));
}

// ------------------------ Timer2 (for input capture) ------------------------
void tim2_init(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    TIM2->PSC = 79;   // 1MHz timer (1 tick = 1us)
    TIM2->ARR = 0xFFFF;
    TIM2->CCMR1 &= ~0x3F;
    TIM2->CCMR1 |= 1;            // CC1S = 01 -> TI1 input
    TIM2->CCER &= ~(0xF);
    TIM2->CCER |= 1;             // capture rising edge
    TIM2->CR1 |= 1;              // enable timer
}

uint16_t measure_distance(void) {
    uint32_t rising=0, falling=0;

    // send 10us pulse
    TRIG_PORT->ODR |= (1<<TRIG_PIN);
    delay_us(10);
    TRIG_PORT->ODR &= ~(1<<TRIG_PIN);

    // wait for rising edge
    TIM2->CCER &= ~(1<<1);   // capture rising
    TIM2->SR &= ~(1<<1);     // clear flag
    while(!(TIM2->SR & 1));
    rising = TIM2->CCR1;
    TIM2->SR &= ~1;

    // falling edge
    TIM2->CCER |= (1<<1);
    while(!(TIM2->SR & 1));
    falling = TIM2->CCR1;
    TIM2->SR &= ~1;

    // compute pulse width
    uint32_t pulse = (falling >= rising) ? (falling - rising) : ((0xFFFF - rising) + falling);
    return pulse * 0.034 / 2;  // distance in cm
}

// ------------------------ Main ------------------------
int main(void) {
    gpio_init();
    uart_init();
    tim2_init();

    char msg[50];
    while(1) {
        uint16_t dist = measure_distance();
        sprintf(msg, "Distance = %u cm\r\n", dist);
        uart_send_str(msg);
        delay_ms(200);
    }
}
