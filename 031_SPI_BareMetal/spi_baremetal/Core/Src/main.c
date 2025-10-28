#include "stm32l476xx.h"
#include "spi.h"
#include "uart.h"

int main(void)
{
    UART2_Init();
    SPI1_Master_Init();

    uint8_t tx_data = 0x30;
    uint8_t rx_data;

    UART2_SendString("STM32 SPI Master Started\r\n");

    while (1)
    {
        rx_data = SPI1_TransmitReceive(tx_data);

        UART2_SendString("Sent: ");
        UART2_SendHex(tx_data);
        UART2_SendString(" | Received: ");
        UART2_SendHex(rx_data);
        UART2_SendString("\r\n");

        tx_data++;
        if (tx_data > 0x39) tx_data = 0x30;

        for (volatile int i = 0; i < 1000000; i++); // delay ~1s
    }
}
