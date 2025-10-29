#include "main.h"

#define LED_PIN                 GPIO_PIN_5
#define LED_PORT                GPIOA
#define BUTTON_PIN              GPIO_PIN_13
#define BUTTON_PORT             GPIOC

#define DEBOUNCE_DELAY_MS       50
#define LONG_PRESS_TIME_MS      2000

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void delay_ms(uint32_t ms);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    uint8_t ledState = 0;
    uint32_t pressStart = 0;
    uint8_t buttonPressed = 0;

    while (1)
    {
        // Read button (active low)
        if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
        {
            HAL_Delay(DEBOUNCE_DELAY_MS); // debounce

            if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
            {
                if (!buttonPressed)
                {
                    buttonPressed = 1;
                    pressStart = HAL_GetTick();
                }
            }
        }
        else
        {
            if (buttonPressed)
            {
                uint32_t pressDuration = HAL_GetTick() - pressStart;
                buttonPressed = 0;

                if (pressDuration >= LONG_PRESS_TIME_MS)
                {
                    // Long press: blink 3 times rapidly
                    for (int i = 0; i < 3; i++)
                    {
                        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
                        HAL_Delay(150);
                        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
                        HAL_Delay(150);
                    }
                }
                else
                {
                    // Short press: toggle LED
                    ledState = !ledState;
                    HAL_GPIO_WritePin(LED_PORT, LED_PIN, ledState ? GPIO_PIN_SET : GPIO_PIN_RESET);
                }
            }
        }
    }
}

/* --- Helper Functions --- */
void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/* --- GPIO Initialization --- */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // LED PA5
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    // Button PC13
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
}

/* --- System Clock Config --- */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}
