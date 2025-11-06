#include "stm32l476xx.h"
#include <stdio.h>
#include <string.h>

#define TRIG_PIN    0   // PB0
#define ECHO_PIN    1   // PB1
#define CS_LOW()    (GPIOB->BSRR = (1 << (6 + 16)))
#define CS_HIGH()   (GPIOB->BSRR = (1 << 6))

// =========================================================
// Simple delay (approx microseconds)
// =========================================================
void delay_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 4; i++);
}

// =========================================================
// UART2 Init and Print
// =========================================================
void UART2_Init(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |=  (2 << (2*2)) | (2 << (3*2));  // AF
    GPIOA->AFR[0] |= (7 << (4*2)) | (7 << (4*3));  // AF7

    USART2->BRR = 4000000 / 9600;  // 4MHz / 9600 baud
    USART2->CR1 = (1 << 3) | (1 << 2) | (1 << 0);  // TE, RE, UE
}

void uart2_putc(char c) {
    while (!(USART2->ISR & (1 << 7)));
    USART2->TDR = c;
}

void UART2_SendString(char *s) {
    while (*s) uart2_putc(*s++);
}

// =========================================================
// SPI1 Init + Basic R/W
// =========================================================
void SPI1_Init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // PA5=SCK, PA6=MISO, PA7=MOSI
    GPIOA->MODER &= ~(0x3F << 10);
    GPIOA->MODER |=  (0x2A << 10);   // AF mode
    GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28);  // AF5

    // PB6 = CS
    GPIOB->MODER &= ~(3 << (6*2));
    GPIOB->MODER |=  (1 << (6*2));
    GPIOB->BSRR = (1 << 6);

    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | (3 << 3); // fPCLK/16
    SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t SPI1_Transfer(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));
    *((__IO uint8_t *)&SPI1->DR) = data;
    while (!(SPI1->SR & SPI_SR_RXNE));
    return *((__IO uint8_t *)&SPI1->DR);
}

// =========================================================
// W25Q Flash Commands
// =========================================================
uint8_t W25Q_ReadID(void) {
    uint8_t id = 0;
    CS_LOW();
    SPI1_Transfer(0x9F);  // JEDEC ID
    SPI1_Transfer(0x00);
    SPI1_Transfer(0x00);
    id = SPI1_Transfer(0x00);
    CS_HIGH();
    return id;
}

void W25Q_WriteEnable(void) {
    CS_LOW();
    SPI1_Transfer(0x06);
    CS_HIGH();
}

void W25Q_PageProgram(uint32_t addr, uint8_t *data, uint16_t len) {
    W25Q_WriteEnable();
    CS_LOW();
    SPI1_Transfer(0x02);
    SPI1_Transfer((addr >> 16) & 0xFF);
    SPI1_Transfer((addr >> 8) & 0xFF);
    SPI1_Transfer(addr & 0xFF);
    for (int i = 0; i < len; i++)
        SPI1_Transfer(data[i]);
    CS_HIGH();
    delay_us(10000);
}

void W25Q_ReadData(uint32_t addr, uint8_t *buf, uint16_t len) {
    CS_LOW();
    SPI1_Transfer(0x03);
    SPI1_Transfer((addr >> 16) & 0xFF);
    SPI1_Transfer((addr >> 8) & 0xFF);
    SPI1_Transfer(addr & 0xFF);
    for (int i = 0; i < len; i++)
        buf[i] = SPI1_Transfer(0x00);
    CS_HIGH();
}

// =========================================================
// Ultrasonic Distance Measure (PB0=Trig, PB1=Echo)
// =========================================================
void Ultrasonic_Init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    GPIOB->MODER &= ~((3 << (2*TRIG_PIN)) | (3 << (2*ECHO_PIN)));
    GPIOB->MODER |=  (1 << (2*TRIG_PIN)); // Output
    GPIOB->PUPDR |=  (1 << (2*ECHO_PIN)); // Pull-up
}

uint32_t Ultrasonic_Read(void) {
    uint32_t count = 0;

    // Send Trigger
    GPIOB->BSRR = (1 << TRIG_PIN);
    delay_us(10);
    GPIOB->BSRR = (1 << (TRIG_PIN + 16));

    // Wait for echo start
    while (!(GPIOB->IDR & (1 << ECHO_PIN))) {
        count++;
        if (count > 1000000) return 65535; // timeout
    }

    // Measure echo high
    count = 0;
    while (GPIOB->IDR & (1 << ECHO_PIN)) {
        count++;
        delay_us(1);
        if (count > 60000) break;
    }

    return (count / 58);  // Convert to cm
}

// =========================================================
// Main
// =========================================================
int main(void) {
    UART2_Init();
    SPI1_Init();
    Ultrasonic_Init();

    UART2_SendString("\r\nSTM32L476RG + W25Q + Ultrasonic\r\n");

    uint8_t read_buf[16];
    uint8_t write_data[] = "HELLO";

    while (1) {
        uint32_t dist = Ultrasonic_Read();
        char msg[64];
        sprintf(msg, "Measured Distance: %lu cm\r\n", dist);
        UART2_SendString(msg);

        // Write ultrasonic distance to flash
        write_data[0] = (uint8_t)dist;
        W25Q_PageProgram(0x000000, write_data, 5);

        // Read back
        W25Q_ReadData(0x000000, read_buf, 5);
        sprintf(msg, "Flash Data: %s\r\n", read_buf);
        UART2_SendString(msg);

        delay_us(500000);
    }
}
