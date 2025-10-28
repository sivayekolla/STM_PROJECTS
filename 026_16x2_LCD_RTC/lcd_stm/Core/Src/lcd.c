#include "lcd.h"

#define RS_PIN   0
#define EN_PIN   1
#define D4_PIN   2
#define D5_PIN   3
#define D6_PIN   4
#define D7_PIN   5
#define LCD_PORT GPIOB

static void delay_us(uint32_t us)
{
    for(uint32_t i = 0; i < us * 4; i++) __NOP();
}

static void pulse_enable(void)
{
    LCD_PORT->BSRR = (1 << EN_PIN);
    delay_us(1);
    LCD_PORT->BRR = (1 << EN_PIN);
    delay_us(100);
}

static void lcd_send_nibble(uint8_t nibble)
{
    LCD_PORT->BRR = (0xF << D4_PIN);
    LCD_PORT->BSRR = ((nibble & 0xF) << D4_PIN);
    pulse_enable();
}

static void lcd_send_cmd(uint8_t cmd)
{
    LCD_PORT->BRR = (1 << RS_PIN);
    lcd_send_nibble(cmd >> 4);
    lcd_send_nibble(cmd & 0x0F);
    delay_us(50);
}

static void lcd_send_data(uint8_t data)
{
    LCD_PORT->BSRR = (1 << RS_PIN);
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
    delay_us(50);
}

void LCD_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    LCD_PORT->MODER &= ~(0x3FFFF);
    LCD_PORT->MODER |=  (0x15555);

    delay_us(15000);
    lcd_send_nibble(0x03);
    delay_us(5000);
    lcd_send_nibble(0x03);
    delay_us(200);
    lcd_send_nibble(0x03);
    lcd_send_nibble(0x02);

    lcd_send_cmd(0x28); // 4-bit, 2-line, 5x8
    lcd_send_cmd(0x0C); // Display ON
    lcd_send_cmd(0x06); // Auto increment
    lcd_send_cmd(0x01); // Clear
    delay_us(2000);
}

void LCD_Clear(void)
{
    lcd_send_cmd(0x01);
    delay_us(2000);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    lcd_send_cmd(0x80 | (addr + col));
}

void LCD_Print(char *str)
{
    while(*str)
    {
        lcd_send_data(*str++);
    }
}
