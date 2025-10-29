#include "ultrasonic_gpio.h"

static void delay_us(uint32_t us) {
    for (uint32_t i = 0; i < us * 4; i++) __NOP();
}

void Ultrasonic_Init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // TRIG = Output
    TRIG_PORT->MODER &= ~(3 << (2 * TRIG_PIN));
    TRIG_PORT->MODER |=  (1 << (2 * TRIG_PIN));

    // ECHO = Input
    ECHO_PORT->MODER &= ~(3 << (2 * ECHO_PIN));
}

uint32_t Ultrasonic_Read(void) {
    uint32_t count = 0;

    // Send trigger pulse
    TRIG_PORT->ODR &= ~(1 << TRIG_PIN);
    delay_us(2);
    TRIG_PORT->ODR |= (1 << TRIG_PIN);
    delay_us(10);
    TRIG_PORT->ODR &= ~(1 << TRIG_PIN);

    // Wait for ECHO HIGH
    while(!(ECHO_PORT->IDR & (1 << ECHO_PIN)));
    while(ECHO_PORT->IDR & (1 << ECHO_PIN)) {
        count++;
        delay_us(1);
    }

    // Convert to cm (approx)
    return (count / 58);
}
