#include "w25qxx.h"

#define W25QXX_CMD_READ_DATA  0x03
#define W25QXX_CMD_WRITE_EN   0x06
#define W25QXX_CMD_PAGE_PROG  0x02
#define W25QXX_CS_LOW()       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define W25QXX_CS_HIGH()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)

SPI_HandleTypeDef *hspi_flash;

void W25QXX_Init(SPI_HandleTypeDef *hspi)
{
    hspi_flash = hspi;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    W25QXX_CS_HIGH();
}

void W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t size)
{
    uint8_t cmd[4];
    cmd[0] = W25QXX_CMD_READ_DATA;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    W25QXX_CS_LOW();
    HAL_SPI_Transmit(hspi_flash, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(hspi_flash, buf, size, HAL_MAX_DELAY);
    W25QXX_CS_HIGH();
}
