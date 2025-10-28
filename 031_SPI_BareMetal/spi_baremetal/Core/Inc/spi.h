#ifndef SPI_H
#define SPI_H

#include "stm32l476xx.h"
#include <stdint.h>

void SPI1_Master_Init(void);
uint8_t SPI1_TransmitReceive(uint8_t data);

#endif
