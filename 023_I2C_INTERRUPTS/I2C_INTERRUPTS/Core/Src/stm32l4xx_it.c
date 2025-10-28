#include "main.h"
#include "stm32l4xx_it.h"
#include "i2c.h"


extern I2C_HandleTypeDef hi2c1;

void SysTick_Handler(void)
{
  HAL_IncTick();
}

void I2C1_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&hi2c1);
}

void I2C1_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&hi2c1);
}
