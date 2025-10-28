#include "i2c_driver.h"

volatile uint8_t i2c_tx_buffer[10];
volatile uint8_t i2c_tx_len = 0;
volatile uint8_t i2c_tx_idx = 0;
volatile uint8_t i2c_busy = 0;

void I2C1_Init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    GPIOB->MODER &= ~((3<<(8*2)) | (3<<(9*2)));
    GPIOB->MODER |=  (2<<(8*2)) | (2<<(9*2));
    GPIOB->AFR[1]  |= (4<<((8-8)*4)) | (4<<((9-8)*4));
    GPIOB->OTYPER |= (1<<8)|(1<<9);
    GPIOB->PUPDR  |= (1<<(8*2)) | (1<<(9*2));

    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    RCC->APB1RSTR1 |= RCC_APB1RSTR1_I2C1RST;
    RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_I2C1RST;

    I2C1->TIMINGR = 0x20303E5D;
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_EnableIRQ(I2C1_ER_IRQn);
    I2C1->CR1 |= I2C_CR1_PE;
}
