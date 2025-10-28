#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <stdarg.h>

SemaphoreHandle_t myMutex;

/* -------------------- Debug print via ITM -------------------- */
void debug_printf(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (int i = 0; buf[i] != '\0'; i++)
        ITM_SendChar(buf[i]);
}

/* -------------------- Task 1 -------------------- */
void Task1(void *param)
{
    for (;;)
    {
        if (xSemaphoreTake(myMutex, portMAX_DELAY) == pdTRUE)
        {
            debug_printf("Task 1 running\r\n");
            xSemaphoreGive(myMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* -------------------- Task 2 -------------------- */
void Task2(void *param)
{
    for (;;)
    {
        if (xSemaphoreTake(myMutex, portMAX_DELAY) == pdTRUE)
        {
            debug_printf("Task 2 running\r\n");
            xSemaphoreGive(myMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(700));
    }
}

/* -------------------- System Clock Config -------------------- */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_Init();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
}

/* -------------------- Main -------------------- */
int main(void)
{
    debug_printf("FreeRTOS Mutex Test\r\n");
    debug_printf("Starting scheduler...\r\n");

    HAL_Init();
    SystemClock_Config();

    debug_printf("FreeRTOS Mutex Test\r\n");

    myMutex = xSemaphoreCreateMutex();
    if (myMutex == NULL)
    {
        debug_printf("Mutex creation failed!\r\n");
        while (1);
    }

    xTaskCreate(Task1, "Task1", 128, NULL, 2, NULL);
    xTaskCreate(Task2, "Task2", 128, NULL, 2, NULL);

    debug_printf("Starting scheduler...\r\n");
    vTaskStartScheduler();

    debug_printf("Scheduler failed!\r\n");
    while (1);
}

/* -------------------- Error Handler -------------------- */
void Error_Handler(void)
{
    __disable_irq();
    while (1);
}
