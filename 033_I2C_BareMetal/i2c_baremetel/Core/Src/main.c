#include "stm32l4xx.h"

#define SLAVE_ADDR 0x28 // ESP32 I2C address

void delay_ms(uint32_t ms);
void I2C1_Init(void);
void I2C1_Send(uint8_t data);

int main(void)
{
    I2C1_Init();

    while (1)
    {
        I2C1_Send('A'); // send one byte
        delay_ms(1000);
    }
}

void I2C1_Init(void)
{
    // Enable GPIOB and I2C1 clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    // Configure PB6 -> SCL, PB7 -> SDA (AF4)
    GPIOB->MODER &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB->MODER |= ((2 << (6 * 2)) | (2 << (7 * 2)));  // AF mode
    GPIOB->OTYPER |= (1 << 6) | (1 << 7);               // Open-drain
    GPIOB->OSPEEDR |= (3 << (6 * 2)) | (3 << (7 * 2));  // High speed
    GPIOB->PUPDR &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB->PUPDR |= ((1 << (6 * 2)) | (1 << (7 * 2)));  // Pull-up
    GPIOB->AFR[0] |= (4 << (6 * 4)) | (4 << (7 * 4));   // AF4 = I2C1

    // Reset I2C1
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Configure timing register (for 100kHz @16MHz)
    I2C1->TIMINGR = 0x00303D5B;

    // Enable I2C1 peripheral
    I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_Send(uint8_t data)
{
    // Start + Write 1 byte
    I2C1->CR2 = (SLAVE_ADDR << 1) | (1 << 16) | (1 << 13); // 1 byte, write, start

    // Wait until TXIS flag set
    while (!(I2C1->ISR & I2C_ISR_TXIS));

    I2C1->TXDR = data; // Write data

    // Wait for TC (Transfer Complete)
    while (!(I2C1->ISR & I2C_ISR_TC));

    // Stop condition
    I2C1->CR2 |= I2C_CR2_STOP;
}

void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 4000; i++) __NOP();
}
