#include "stm32l4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#define GPIOAEN     (1U<<0)
#define GPIOBEN     (1U<<1)
#define USART2EN    (1U<<17)
#define TIM2EN      (1U<<0)
#define TIM1EN      (1U<<11)

#define TRIG_PIN    (1U<<9)
#define ECHO_PIN    (1U<<8)
#define LED_PIN     (1U<<5)
#define RED_PIN     (1U<<0)
#define GREEN_PIN   (1U<<1)
#define BLUE_PIN    (1U<<2)

QueueHandle_t distQueue;

/********** Function Prototypes **********/
void uart2_init(void);
void uart2_write(char ch);
void uart2_write_str(char *str);
void led_init(void);
void trig_init(void);
void rgb_init(void);
void echo_timer_init(void);
uint32_t measure_echo(void);
void delay_us(uint32_t us);

/********** Tasks **********/
void Task_TriggerSensor(void *pv);
void Task_CalcDistance(void *pv);
void Task_UART_Send(void *pv);
void Task_RGB_Update(void *pv);
void Task_LED(void *pv);

/********** Main **********/
int main(void)
{
    RCC->AHB2ENR |= GPIOAEN | GPIOBEN;
    RCC->APB1ENR1 |= USART2EN | TIM2EN;
    RCC->APB2ENR |= TIM1EN;

    uart2_init();
    led_init();
    trig_init();
    rgb_init();
    echo_timer_init();

    distQueue = xQueueCreate(5, sizeof(float));

    xTaskCreate(Task_TriggerSensor, "Trigger", 128, NULL, 3, NULL);
    xTaskCreate(Task_CalcDistance, "Measure", 128, NULL, 3, NULL);
    xTaskCreate(Task_UART_Send, "UART", 128, NULL, 2, NULL);
    xTaskCreate(Task_RGB_Update, "RGB", 128, NULL, 2, NULL);
    xTaskCreate(Task_LED, "LED", 64, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1);
}

/********** LED Init **********/
void led_init(void)
{
    GPIOA->MODER &= ~(3U<<(2*5));
    GPIOA->MODER |=  (1U<<(2*5));
}

/********** RGB LED Init **********/
void rgb_init(void)
{
    GPIOB->MODER &= ~((3U<<(2*0))|(3U<<(2*1))|(3U<<(2*2)));
    GPIOB->MODER |=  ((1U<<(2*0))|(1U<<(2*1))|(1U<<(2*2)));
}

/********** Trigger Init **********/
void trig_init(void)
{
    GPIOA->MODER &= ~(3U<<(2*9));
    GPIOA->MODER |=  (1U<<(2*9));
}

/********** Echo Timer Init **********/
void echo_timer_init(void)
{
    GPIOA->MODER &= ~(3U<<(2*8));
    GPIOA->MODER |=  (0U<<(2*8)); // Input mode
    TIM1->PSC = 80 - 1;  // 1MHz timer => 1µs resolution
    TIM1->CR1 = 1;
}

/********** UART Init **********/
void uart2_init(void)
{
    GPIOA->MODER &= ~((3U<<(2*2))|(3U<<(2*3)));
    GPIOA->MODER |=  ((2U<<(2*2))|(2U<<(2*3)));
    GPIOA->AFR[0] |= (7U<<(4*2))|(7U<<(4*3));

    USART2->BRR = 0x1A0; // 115200 baud @ 80MHz
    USART2->CR1 = (1U<<3)|(1U<<2)|(1U<<0);
}

void uart2_write(char ch)
{
    while(!(USART2->ISR & (1U<<7)));
    USART2->TDR = ch;
}

void uart2_write_str(char *str)
{
    while(*str) uart2_write(*str++);
}

/********** Microsecond Delay **********/
void delay_us(uint32_t us)
{
    TIM2->PSC = 80 - 1;   // 1 MHz
    TIM2->ARR = us;
    TIM2->CNT = 0;
    TIM2->CR1 |= 1;
    while(!(TIM2->SR & 1));
    TIM2->SR = 0;
}

/********** Measure Echo Duration **********/
uint32_t measure_echo(void)
{
    while(!(GPIOA->IDR & ECHO_PIN)); // wait for high
    TIM1->CNT = 0;
    while(GPIOA->IDR & ECHO_PIN);    // wait for low
    return TIM1->CNT;
}

/********** Tasks **********/
void Task_TriggerSensor(void *pv)
{
    while(1)
    {
        GPIOA->BSRR = TRIG_PIN;
        delay_us(10);
        GPIOA->BRR = TRIG_PIN;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Task_CalcDistance(void *pv)
{
    float distance;
    uint32_t echo_time;
    while(1)
    {
        echo_time = measure_echo();
        distance = (echo_time * 0.0343f) / 2.0f;
        xQueueSend(distQueue, &distance, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Task_UART_Send(void *pv)
{
    float dist;
    char buf[50];
    while(1)
    {
        if(xQueueReceive(distQueue, &dist, portMAX_DELAY) == pdPASS)
        {
            sprintf(buf, "Distance: %.2f cm\r\n", dist);
            uart2_write_str(buf);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Task_RGB_Update(void *pv)
{
    float dist;
    while(1)
    {
        if(xQueuePeek(distQueue, &dist, 0) == pdPASS)
        {
            if(dist < 10)
            {
                GPIOB->ODR = RED_PIN;
            }
            else if(dist < 20)
            {
                GPIOB->ODR = GREEN_PIN;
            }
            else
            {
                GPIOB->ODR = BLUE_PIN;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Task_LED(void *pv)
{
    while(1)
    {
        GPIOA->ODR ^= LED_PIN;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
