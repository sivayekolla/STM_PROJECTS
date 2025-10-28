#include "stm32l476xx.h"

/* --- Function Prototypes --- */
void SystemClock_Config(void);
void UART2_Init(void);
void UART2_SendString(char *str);
void GPIO_Button_Init(void);
void EXTI15_10_Init(void);
void SPI1_Init(void);
void SPI1_SendData(uint8_t *data, uint16_t size);

/* --- MAIN FUNCTION --- */
int main(void)
{
    SystemClock_Config();
    UART2_Init();
    SPI1_Init();
    GPIO_Button_Init();
    EXTI15_10_Init();

    while (1)
    {
        __WFI(); // Sleep until button interrupt
    }
}

/* --- System Clock: 16 MHz HSI --- */
void SystemClock_Config(void)
{
    RCC->CR |= RCC_CR_HSION;                 // Enable HSI
    while (!(RCC->CR & RCC_CR_HSIRDY));      // Wait for HSI ready
    RCC->CFGR &= ~RCC_CFGR_SW;               // Select HSI as system clock
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
}

/* --- UART2 Init (PA2=TX, PA3=RX) --- */
void UART2_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;     // Enable GPIOA
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;  // Enable USART2

    // Alternate function for PA2, PA3
    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |=  (2 << (2*2)) | (2 << (3*2));
    GPIOA->AFR[0]  &= ~((0xF << (4*2)) | (0xF << (4*3)));
    GPIOA->AFR[0]  |=  (7 << (4*2)) | (7 << (4*3));

    USART2->BRR = 16000000 / 115200;         // Baud rate
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE; // Enable TX, USART
}

/* --- UART Send String --- */
void UART2_SendString(char *str)
{
    while (*str)
    {
        while (!(USART2->ISR & USART_ISR_TXE));
        USART2->TDR = *str++;
    }
    while (!(USART2->ISR & USART_ISR_TC));
}

/* --- SPI1 Init (PA5=SCK, PA6=MISO, PA7=MOSI, PA4=NSS) --- */
void SPI1_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Alternate Function Mode
    GPIOA->MODER &= ~((3<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2)));
    GPIOA->MODER |=  (2<<(4*2)) | (2<<(5*2)) | (2<<(6*2)) | (2<<(7*2));
    GPIOA->AFR[0] &= ~((0xF<<(4*4)) | (0xF<<(4*5)) | (0xF<<(4*6)) | (0xF<<(4*7)));
    GPIOA->AFR[0] |=  (5<<(4*4)) | (5<<(4*5)) | (5<<(4*6)) | (5<<(4*7));

    // SPI1 Config: Master, SSM, SSI, fPCLK/8
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1;
    SPI1->CR1 |= SPI_CR1_SPE;
}

/* --- SPI1 Send Data --- */
void SPI1_SendData(uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        while (!(SPI1->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&SPI1->DR) = data[i];
    }
    while (SPI1->SR & SPI_SR_BSY);
}

/* --- Button GPIO Init (PC13) --- */
void GPIO_Button_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    GPIOC->MODER &= ~(3 << (13 * 2));  // Input
}

/* --- External Interrupt Init for PC13 --- */
void EXTI15_10_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[3] &= ~(0xF << 4);
    SYSCFG->EXTICR[3] |=  (0x2 << 4); // Map PC13 to EXTI13

    EXTI->IMR1  |= (1 << 13);  // Unmask EXTI13
    EXTI->FTSR1 |= (1 << 13);  // Falling edge trigger

    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 1);
}

/* --- Interrupt Handler for PC13 Button --- */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << 13))
    {
        EXTI->PR1 = (1 << 13); // Clear pending flag

        UART2_SendString("Hi\r\n");

        uint8_t msg[] = "Data Transfer Done\r\n";
        SPI1_SendData(msg, sizeof(msg) - 1);
    }
}
