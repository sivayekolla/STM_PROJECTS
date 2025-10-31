#include <stdint.h>
#include <stdio.h>

#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL
#define I2C1_BASE       0x40005400UL
#define USART2_BASE     0x40004400UL

#define RCC_AHB2ENR     (*(volatile uint32_t*)(RCC_BASE + 0x4C))
#define RCC_APB1ENR1    (*(volatile uint32_t*)(RCC_BASE + 0x58))

#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t*)(GPIOA_BASE + 0x20))

#define I2C1_CR1        (*(volatile uint32_t*)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t*)(I2C1_BASE + 0x04))
#define I2C1_ISR        (*(volatile uint32_t*)(I2C1_BASE + 0x18))
#define I2C1_TXDR       (*(volatile uint32_t*)(I2C1_BASE + 0x28))
#define I2C1_RXDR       (*(volatile uint32_t*)(I2C1_BASE + 0x24))
#define I2C1_TIMINGR    (*(volatile uint32_t*)(I2C1_BASE + 0x10))
#define USART2_CR1      (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_BRR      (*(volatile uint32_t*)(USART2_BASE + 0x0C))
#define USART2_ISR      (*(volatile uint32_t*)(USART2_BASE + 0x1C))
#define USART2_TDR      (*(volatile uint32_t*)(USART2_BASE + 0x28))

static void delay_ms(uint32_t ms) { for(uint32_t i=0;i<ms*4000;i++) __asm__("nop"); }



static void uart2_init(void) {
    RCC_AHB2ENR |= (1<<0);
    RCC_APB1ENR1 |= (1<<17);
    GPIOA_MODER &= ~(3<<(2*2));
    GPIOA_MODER |=  (2<<(2*2));
    GPIOA_AFRL  |=  (7<<(4*2));
    USART2_BRR = 4000000/115200;
    USART2_CR1 = (1<<3)|(1<<0);
}

static void uart2_write(char c) { while(!(USART2_ISR&(1<<7))); USART2_TDR = c; }
static void uart2_print(char*s){ while(*s) uart2_write(*s++); }


static void i2c1_init(void){
    RCC_AHB2ENR |= (1<<0);
    RCC_APB1ENR1 |= (1<<21);
    I2C1_CR1 &= ~(1<<0);
    I2C1_TIMINGR = 0x00303D5B;
    I2C1_CR1 |= (1<<0);
}

static void i2c1_write(uint8_t addr, uint8_t reg, uint16_t data){
    while(I2C1_ISR & (1<<15));
    I2C1_CR2 = (addr<<1)|(3<<16);
    I2C1_CR2 |= (1<<13);
    while(!(I2C1_ISR&(1<<1)));
    I2C1_TXDR = reg;
    while(!(I2C1_ISR&(1<<1)));
    I2C1_TXDR = (data>>8)&0xFF;
    while(!(I2C1_ISR&(1<<1)));
    I2C1_TXDR = data & 0xFF;
    while(!(I2C1_ISR&(1<<6)));
}

static uint16_t i2c1_read(uint8_t addr, uint8_t reg){
    uint16_t val;
    while(I2C1_ISR & (1<<15));
    I2C1_CR2 = (addr<<1)|(1<<13);
    while(!(I2C1_ISR&(1<<1)));
    I2C1_TXDR = reg;
    while(!(I2C1_ISR&(1<<6)));
    I2C1_CR2 = (addr<<1)|(1<<10)|(2<<16)|(1<<13);
    while(!(I2C1_ISR&(1<<2)));
    val = I2C1_RXDR<<8;
    while(!(I2C1_ISR&(1<<2)));
    val |= I2C1_RXDR;
    while(!(I2C1_ISR&(1<<6)));
    return val;
}


#define OPT3001_ADDR 0x47
#define OPT3001_CONFIG_REG 0x01
#define OPT3001_RESULT_REG 0x00

static void opt3001_init(void){
    i2c1_write(OPT3001_ADDR, OPT3001_CONFIG_REG, 0xC410);
}

static float opt3001_read_lux(void){
    uint16_t raw = i2c1_read(OPT3001_ADDR, OPT3001_RESULT_REG);
    uint16_t exp = (raw>>12)&0xF;
    uint16_t mant = raw&0xFFF;
    float lux = (0.01 * (1<<exp) * mant);
    return lux;
}


int main(void){
    uart2_init();
    i2c1_init();
    opt3001_init();
    delay_ms(100);

    char buf[64];
    while(1){
        float lux = opt3001_read_lux();
        snprintf(buf,sizeof(buf),"Ambient Light: %.2f lux\r\n",lux);
        uart2_print(buf);
        delay_ms(1000);
    }
}
