#ifndef W25QXX_H
#define W25QXX_H
#include "stm32l4xx_hal.h"

void W25QXX_Init(SPI_HandleTypeDef *hspi);
void W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t size);
void W25QXX_Write(uint8_t *buf, uint32_t addr, uint16_t size);

#endif
