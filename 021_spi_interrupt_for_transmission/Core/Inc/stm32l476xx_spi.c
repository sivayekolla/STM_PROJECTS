#include "stm32l476xx_spi.h"


void SPI_Init(SPI_RegDef_t *pSPIx, uint32_t mode, uint32_t buscfg,
              uint32_t baud, uint32_t ssm_enable)
{
	RCC->APB2ENR |= (1<<12);
    uint32_t temp = 0;

    temp |= mode;
    temp |= buscfg;
    temp |= baud;
    temp |= ssm_enable;

    if (ssm_enable == SPI_SSM_ENABLE)
        temp |= SPI_SSI_SET;

    pSPIx->CR1 = temp;

    pSPIx->CR1 |= (1U << SPI_CR1_SPE_Pos);
}

void SPI1_GPIO_Config(void)
{

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER &= ~((3U << (4 * 2)) | (3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA->MODER |=  ((2U << (4 * 2)) | (2U << (5 * 2)) | (2U << (6 * 2)) | (2U << (7 * 2)));

    GPIOA->AFR[0] &= ~((0xFU << (4 * 4)) | (0xFU << (5 * 4)) | (0xFU << (6 * 4)) | (0xFU << (7 * 4)));
    GPIOA->AFR[0] |=  ((5U << (4 * 4)) | (5U << (5 * 4)) | (5U << (6 * 4)) | (5U << (7 * 4)));

    GPIOA->OSPEEDR |=  ((3U << (4 * 2)) | (3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA->OTYPER  &= ~((1U << 4) | (1U << 5) | (1U << 6) | (1U << 7));
    GPIOA->PUPDR   &= ~((3U << (4 * 2)) | (3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
}

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi)
        pSPIx->CR1 |= (1U << SPI_CR1_SPE_Pos);
    else
        pSPIx->CR1 &= ~(1U << SPI_CR1_SPE_Pos);
}
void SPI_TransmitReceive(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint8_t *pRxBuffer, uint32_t len)
{
    while (len > 0)
    {
        while (!(pSPIx->SR & (1U << 1)));
        pSPIx->DR = *pTxBuffer;
        pTxBuffer++;
        while (!(pSPIx->SR & (1U << 0)));
        *pRxBuffer = pSPIx->DR;
        pRxBuffer++;
        len--;
    }
    while (pSPIx->SR & (1U << 7));
}
