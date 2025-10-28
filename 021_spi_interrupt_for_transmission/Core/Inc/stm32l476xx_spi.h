#ifndef STM32L476XX_SPI_H
#define STM32L476XX_SPI_H

#include <stdint.h>
#include "stm32l476xx.h"
#define PERIPH_BASEADDR       0x40000000U
#define APB1PERIPH_BASE       PERIPH_BASEADDR
#define APB2PERIPH_BASE       (PERIPH_BASEADDR + 0x00010000U)

#define SPI1_BASEADDR         (APB2PERIPH_BASE + 0x3000U)
#define SPI2_BASEADDR         (APB1PERIPH_BASE + 0x3800U)
#define SPI3_BASEADDR         (APB1PERIPH_BASE + 0x3C00U)

typedef struct
{
    volatile uint32_t CR1;      // Control register 1
    volatile uint32_t CR2;      // Control register 2
    volatile uint32_t SR;       // Status register
    volatile uint32_t DR;       // Data register
    volatile uint32_t CRCPR;    // CRC polynomial register
    volatile uint32_t RXCRCR;   // RX CRC register
    volatile uint32_t TXCRCR;   // TX CRC register
    volatile uint32_t I2SCFGR;  // I2S configuration register
    volatile uint32_t I2SPR;
} SPI_RegDef_t;
#if 1
#define SPI_1   ((SPI_RegDef_t *)SPI1_BASEADDR)
#define SPI_2   ((SPI_RegDef_t *)SPI2_BASEADDR)
#define SPI_3   ((SPI_RegDef_t *)SPI3_BASEADDR)

#define SPI_CR1_CPHA_Pos      0
#define SPI_CR1_CPOL_Pos      1
#define SPI_CR1_MSTR_Pos      2
#define SPI_CR1_BR_Pos        3
#define SPI_CR1_SPE_Pos       6
#define SPI_CR1_LSBFIRST_Pos  7
#define SPI_CR1_SSI_Pos       8
#define SPI_CR1_SSM_Pos       9
#define SPI_CR1_RXONLY_Pos    10
#define SPI_CR1_DFF_Pos       11
#define SPI_CR1_BIDIOE_Pos    14
#define SPI_CR1_BIDIMODE_Pos  15
#endif

#define SPI_MODE_MASTER        (1U << SPI_CR1_MSTR_Pos)
#define SPI_MODE_SLAVE         (0U << SPI_CR1_MSTR_Pos)

#define SPI_FULL_DUPLEX        (0U << SPI_CR1_BIDIMODE_Pos)
#define SPI_HALF_DUPLEX        (1U << SPI_CR1_BIDIMODE_Pos)
#define SPI_SIMPLEX_RXONLY     (1U << SPI_CR1_RXONLY_Pos)

#define SPI_SSM_ENABLE         (1U << SPI_CR1_SSM_Pos)
#define SPI_SSM_DISABLE        (0U << SPI_CR1_SSM_Pos)

#define SPI_SSI_SET            (1U << SPI_CR1_SSI_Pos)

#define SPI_BAUD_DIV2          (0U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV4          (1U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV8          (2U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV16         (3U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV32         (4U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV64         (5U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV128        (6U << SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV256        (7U << SPI_CR1_BR_Pos)

void SPI_Init(SPI_RegDef_t *pSPIx, uint32_t mode, uint32_t buscfg,
              uint32_t baud, uint32_t ssm_enable);

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);


void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len);


void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len);
void SPI1_GPIO_Config(void);

#endif
