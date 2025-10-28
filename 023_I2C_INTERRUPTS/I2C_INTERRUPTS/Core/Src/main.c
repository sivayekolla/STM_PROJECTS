#include "main.h"

uint8_t message[] = "Button pressed";

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_I2C1_Init();

  while (1)
  {
    I2C_SendMessage(message, sizeof(message));
    HAL_Delay(2000);
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
