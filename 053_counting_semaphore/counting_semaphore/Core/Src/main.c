#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32l476xx.h"

/*-----------------------------------------------------------*/
/* Simple SWV print using ITM (visible in STM32CubeIDE SWV console) */
static inline void SWV_Print(const char *s)
{
    while (*s)
    {
        ITM_SendChar(*s++);
    }
}

/*-----------------------------------------------------------*/
SemaphoreHandle_t xSem;

/* Sender Task */
void SenderTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        xSemaphoreGive(xSem);
        SWV_Print("Semaphore given\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Receiver Task */
void ReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        if (xSemaphoreTake(xSem, portMAX_DELAY) == pdTRUE)
        {
            SWV_Print("Semaphore taken\r\n");
        }
    }
}

/*-----------------------------------------------------------*/
/* System Clock Setup: use MSI 4MHz + PLL -> 80 MHz */
void SystemClock_Config(void)
{
    /* Enable PWR and set voltage scale 1 */
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR1 &= ~PWR_CR1_VOS;
    PWR->CR1 |= PWR_CR1_VOS_0;

    /* Enable MSI */
    RCC->CR |= RCC_CR_MSION;
    while ((RCC->CR & RCC_CR_MSIRDY) == 0);

    /* Configure MSI range */
    RCC->CR &= ~RCC_CR_MSIRANGE;
    RCC->CR |= RCC_CR_MSIRANGE_6; // 4 MHz
    RCC->CR |= RCC_CR_MSIRGSEL;

    /* Configure PLL (4 MHz * 40 / 2 = 80 MHz) */
    RCC->PLLCFGR = (1 << RCC_PLLCFGR_PLLSRC_Pos) | (1 << RCC_PLLCFGR_PLLM_Pos)
                 | (40 << RCC_PLLCFGR_PLLN_Pos)
                 | (0 << RCC_PLLCFGR_PLLP_Pos)
                 | (0 << RCC_PLLCFGR_PLLQ_Pos)
                 | (0 << RCC_PLLCFGR_PLLR_Pos)
                 | RCC_PLLCFGR_PLLREN;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    /* Switch system clock to PLL */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

/*-----------------------------------------------------------*/
int main(void)
{
    SystemClock_Config();

    SWV_Print("\r\n=== FreeRTOS SWV Semaphore Demo ===\r\n");

    xSem = xSemaphoreCreateBinary();
    if (xSem == NULL)
    {
        SWV_Print("Failed to create semaphore!\r\n");
        while (1);
    }

    xTaskCreate(SenderTask, "Sender", 128, NULL, 1, NULL);
    xTaskCreate(ReceiverTask, "Receiver", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
        /* Should never reach here */
        SWV_Print("Scheduler failed to start!\r\n");
    }
}
