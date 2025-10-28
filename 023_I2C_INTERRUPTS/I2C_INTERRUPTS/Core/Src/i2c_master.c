#include "i2c_driver.h"

void I2C1_Start_Transmit(uint8_t addr, uint8_t *data, uint8_t len) {
    while (i2c_busy);
    i2c_busy = 1;
    i2c_tx_idx = 0;
    i2c_tx_len = len;

    for (uint8_t i = 0; i < len; i++)
        i2c_tx_buffer[i] = data[i];

    I2C1->CR2 = (addr << 1) | (len << 16) | I2C_CR2_AUTOEND | I2C_CR2_START;
}

void I2C1_EV_IRQHandler(void) {
    if (I2C1->ISR & I2C_ISR_TXIS) {
        if (i2c_tx_idx < i2c_tx_len)
            I2C1->TXDR = i2c_tx_buffer[i2c_tx_idx++];
    }
    if (I2C1->ISR & I2C_ISR_TC)
        i2c_busy = 0;
}

void I2C1_ER_IRQHandler(void) {
    I2C1->ICR = 0xFFFFFFFF;
    i2c_busy = 0;
}
