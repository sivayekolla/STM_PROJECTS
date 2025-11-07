#include "stm32l4xx.h"
#include <string.h>
void SystemInit(void)
{
    // minimal system clock setup to avoid linker errors
}

#define I2C_ADDRESS     0x28    // Slave address (must match ESP32)
#define BUTTON_PIN      13      // PC13 user button
#define LED_PIN         5       // PA5 LED

uint8_t txData[] = "Hello ESP32!\r\n";
volatile uint8_t send_flag = 0;

void GPIO_Init(void);
void I2C1_Init(void);
void I2C1_EV_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

int main(void)
{
    // System Init
    SystemInit();
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    GPIO_Init();
    I2C1_Init();

    while (1)
    {
        if (send_flag)
        {
            send_flag = 0;

            // Generate START + Address
            I2C1->CR2 = (I2C_ADDRESS << 1) | (strlen((char*)txData) << 16);
            I2C1->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;

            for (uint8_t i = 0; i < strlen((char*)txData); i++)
            {
                while (!(I2C1->ISR & I2C_ISR_TXIS));
                I2C1->TXDR = txData[i];
            }
            while (!(I2C1->ISR & I2C_ISR_STOPF));
            I2C1->ICR |= I2C_ICR_STOPCF;

            GPIOA->ODR ^= (1 << LED_PIN); // Toggle LED after send
        }

        __WFI(); // Wait for interrupt
    }
}

void GPIO_Init(void)
{
    // LED PA5
    GPIOA->MODER &= ~(3 << (LED_PIN * 2));
    GPIOA->MODER |= (1 << (LED_PIN * 2));

    // I2C1 PB8 (SCL), PB9 (SDA)
    GPIOB->MODER &= ~((3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOB->MODER |= ((2 << (8 * 2)) | (2 << (9 * 2))); // AF mode
    GPIOB->OTYPER |= ((1 << 8) | (1 << 9));             // Open-drain
    GPIOB->OSPEEDR |= ((3 << (8 * 2)) | (3 << (9 * 2))); // High speed
    GPIOB->AFR[1] |= (4 << ((8 - 8) * 4)) | (4 << ((9 - 8) * 4)); // AF4 for I2C1

    // Button PC13
    GPIOC->MODER &= ~(3 << (BUTTON_PIN * 2));
    GPIOC->PUPDR |= (2 << (BUTTON_PIN * 2)); // Pull-down
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;
    EXTI->IMR1 |= (1 << BUTTON_PIN);
    EXTI->FTSR1 |= (1 << BUTTON_PIN);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void I2C1_Init(void)
{
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x10707DBC; // 100kHz @ 16MHz
    I2C1->OAR1 = (1 << 15);     // 7-bit own address mode
    I2C1->CR1 |= I2C_CR1_PE;
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << BUTTON_PIN))
    {
        EXTI->PR1 = (1 << BUTTON_PIN);
        send_flag = 1;
    }
}
