#include "stm32l4xx.h"   // Correct header for STM32L4 series

// Servo PWM parameters
#define SERVO_MIN      500      // 0° -> 0.5 ms
#define SERVO_MAX      2500     // 180° -> 2.5 ms
#define SERVO_PERIOD   20000    // 20 ms period (50 Hz)

volatile uint16_t current_angle = 0;

// Preset angles
#define ANGLE1 30
#define ANGLE2 150

// Simple crude delay (~ms)
void delay_ms(uint32_t ms){
    for(uint32_t i=0;i<ms*4000;i++) __NOP();
}

// Initialize TIM3 PWM on PA6
void servo_init(void){
    // 1. Enable clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;       // GPIOA clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;      // TIM3 clock

    // 2. Configure PA6 as AF2 (TIM3_CH1)
    GPIOA->MODER &= ~(3U << (6*2));   // Clear mode
    GPIOA->MODER |= 2U << (6*2);      // AF mode
    GPIOA->AFR[0] &= ~(0xF << (6*4));
    GPIOA->AFR[0] |= 2 << (6*4);      // AF2 = TIM3

    // 3. Configure TIM3
    TIM3->PSC = 16-1;                  // Prescaler: 16MHz/16 = 1 MHz
    TIM3->ARR = SERVO_PERIOD - 1;      // 20 ms period
    TIM3->CCR1 = SERVO_MIN;            // Initial position

    TIM3->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM3->CCMR1 |= (6 << 4);           // PWM mode 1
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;    // Preload enable

    TIM3->CCER |= TIM_CCER_CC1E;       // Enable channel 1 output
    TIM3->CR1 |= TIM_CR1_CEN;          // Enable counter
}

// Convert angle (0-180°) to PWM pulse width
uint16_t angle_to_pulse(uint16_t angle){
    if(angle > 180) angle = 180;
    return SERVO_MIN + ((SERVO_MAX - SERVO_MIN) * angle) / 180;
}

int main(void){
    servo_init();

    uint32_t t = 0;  // Time counter in seconds
    static uint32_t counter = 0;

    while(1){
        t = t % 20; // 0-19 sec loop

        uint16_t target_angle = (t < 10) ? ANGLE1 : ANGLE2;

        // Smooth move
        if(current_angle < target_angle) current_angle++;
        else if(current_angle > target_angle) current_angle--;

        TIM3->CCR1 = angle_to_pulse(current_angle);

        delay_ms(20); // 50 Hz update

        // Increment time counter every 1 second
        counter++;
        if(counter >= 50){ // 50*20ms ~ 1s
            counter = 0;
            t++;
        }
    }
}
