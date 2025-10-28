#include "stm32l476xx.h"
#include "lcd.h"
#include "rtc.h"
#include <stdio.h>

int main(void)
{
    SystemInit();
    LCD_Init();
    RTC_Init();   // Start RTC (1 Hz tick)

    char time_str[16];
    char date_str[16];
    uint8_t h, m, s, d, mo, y;
    char am_pm;

    LCD_Clear();
    LCD_Print("   HELLO  ");
    for (volatile int i=0; i<500000; i++);  // small delay
    LCD_Clear();

    while (1)
    {
        RTC_GetTime12(&h, &m, &s, &am_pm);
        RTC_GetDate(&d, &mo, &y);

        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d %cM", h, m, s, am_pm);
        snprintf(date_str, sizeof(date_str), "%02d/%02d/20%02d", d, mo, y);

        LCD_Clear();
        LCD_SetCursor(0,0);
        LCD_Print(time_str);
        LCD_SetCursor(1,0);
        LCD_Print(date_str);

        // Wait until RTC seconds register changes
        uint8_t last = s;
        while (last == ((RTC->TR >> 0) & 0x7F));  // block until next second
    }
}
