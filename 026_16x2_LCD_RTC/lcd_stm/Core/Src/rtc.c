#include "rtc.h"

// Initialize RTC with LSE 32.768 kHz clock
void RTC_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR1 |= PWR_CR1_DBP; // enable backup domain access

    RCC->BDCR |= RCC_BDCR_LSEON;
    while(!(RCC->BDCR & RCC_BDCR_LSERDY));

    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;  // LSE as RTC source
    RCC->BDCR |= RCC_BDCR_RTCEN;

    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->ISR |= RTC_ISR_INIT;
    while(!(RTC->ISR & RTC_ISR_INITF));

    RTC->PRER = (127 << 16) | 255;    // 1 Hz clock from 32.768 kHz
    RTC->CR |= RTC_CR_FMT;            // 12 h mode

    // Set initial India time (UTC+5:30)
    RTC->TR = (0x10 << 16) | (0x45 << 8) | (0x00); // 10:45:00
    RTC->DR = (0x25 << 16) | (0x10 << 8) | (0x25); // 25-Oct-2025

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}

void RTC_GetTime12(uint8_t *h, uint8_t *m, uint8_t *s, char *am_pm)
{
    uint32_t tr = RTC->TR;
    *h = ((tr >> 20) & 0x3)*10 + ((tr >> 16) & 0xF);
    *m = ((tr >> 12) & 0x7)*10 + ((tr >> 8) & 0xF);
    *s = ((tr >> 4) & 0x7)*10 + (tr & 0xF);
    *am_pm = (tr & RTC_TR_PM) ? 'P' : 'A';
}

void RTC_GetDate(uint8_t *d, uint8_t *mo, uint8_t *y)
{
    uint32_t dr = RTC->DR;
    *d  = ((dr >> 20)&0x3)*10 + ((dr >>16)&0xF);
    *mo = ((dr >> 12)&0x1)*10 + ((dr >> 8)&0xF);
    *y  = ((dr >> 4)&0xF)*10 + (dr & 0xF);
}
