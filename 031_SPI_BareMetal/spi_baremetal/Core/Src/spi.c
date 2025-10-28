#include "spi.h"

void SPI1_Master_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // PA5=SCK, PA6=MISO, PA7=MOSI, PA4=CS
    GPIOA->MODER &= ~( (3U<<(5*2)) | (3U<<(6*2)) | (3U<<(7*2)) | (3U<<(4*2)) );
    GPIOA->MODER |= ( (2U<<(5*2)) | (2U<<(6*2)) | (2U<<(7*2)) ); // AF mode
    GPIOA->AFR[0] |= (5U<<(5*4)) | (5U<<(6*4)) | (5U<<(7*4)); // AF5 = SPI1
    GPIOA->MODER |= (1U<<(4*2)); // PA4 = output for CS
    GPIOA->ODR |= (1U<<4); // CS high

    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_BR_2;
    SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t SPI1_TransmitReceive(uint8_t data)
{
    GPIOA->ODR &= ~(1U<<4); // CS low

    while (!(SPI1->SR & SPI_SR_TXE));
    *((__IO uint8_t*)&SPI1->DR) = data;

    while (!(SPI1->SR & SPI_SR_RXNE));
    uint8_t recv = *((__IO uint8_t*)&SPI1->DR);

    GPIOA->ODR |= (1U<<4); // CS high
    return recv;
}
