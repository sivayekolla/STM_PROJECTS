#include "main.h"
#include "stm32l4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"
/* USER CODE BEGIN Includes */
#include "stm32l4xx_hal.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define USART_TX_Pin GPIO_PIN_2
#define USART_RX_Pin GPIO_PIN_3
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_GPIO_Port GPIOA
/* USER CODE END Private defines */

void NMI_Handler(void) { while (1); }
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void DebugMon_Handler(void) {}

/* DO NOT redefine SysTick_Handler here — handled by FreeRTOS port.c */
