#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "stm32l476xx.h"

#define I2C_ADDR_ESP32  0x28

extern volatile uint8_t i2c_tx_buffer[10];
extern volatile uint8_t i2c_tx_len;
extern volatile uint8_t i2c_tx_idx;
extern volatile uint8_t i2c_busy;

void I2C1_Init(void);
void I2C1_Start_Transmit(uint8_t addr, uint8_t *data, uint8_t len);

#endif
