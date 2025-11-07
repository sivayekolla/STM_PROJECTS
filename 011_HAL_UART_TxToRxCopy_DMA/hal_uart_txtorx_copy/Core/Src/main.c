#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include <stdio.h>

#define LED_PIN   GPIO_PIN_5
#define LED_PORT  GPIOA

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;

uint8_t tx_buffer[10] = {1,2,3,4,5,6,7,8,9,10};
uint8_t rx_buffer[10];


void uart_init(void);
void led_init(void);
void Error_Handler(void);

int main(void)
{
    HAL_Init();
    uart_init();
    led_init();

    // Start DMA transfer
    if (HAL_UART_Transmit_DMA(&huart1, tx_buffer, 10) != HAL_OK)
        Error_Handler();

    if (HAL_UART_Receive_DMA(&huart1, rx_buffer, 10) != HAL_OK)
        Error_Handler();

    while (1)
    {
        // Blink LED every 500ms to show MCU is alive
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(500);
    }
}

void uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. Enable clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    // 2. Configure PA9 (TX) and PA10 (RX) as Alternate Function
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. UART1 Configuration
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();

    // 4. Configure DMA for UART1_RX
    hdma_usart1_rx.Instance = DMA1_Channel5;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_2;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;

    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
        Error_Handler();

    __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

    // 5. Configure DMA for UART1_TX
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_2;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;

    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
        Error_Handler();

    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

    // 6. Enable DMA interrupt priorities
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    // Enable UART1 interrupt for DMA complete events
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

// DMA IRQ Handlers
void DMA1_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

void DMA1_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

// UART interrupt handler (optional if using DMA complete callbacks)
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

// HAL tick handler
void SysTick_Handler(void)
{
    HAL_IncTick();
}

// Optional: DMA complete callbacks
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);  // blink when TX complete
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);  // blink when RX complete
}

// Error handler
void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(200);
    }
}
