/* -------------------------------------------------------------
 * STM32L476RG  +  BOOSTXL-SENSORS (BMI160)  BARE-METAL DEMO
 * I2C1 : PB8=SCL, PB9=SDA
 * UART2: PA2=TX  (connect to Arduino RX)
 * Power: 3V3 + GND
 * Baud: 115200
 * ------------------------------------------------------------- */
#include <stdint.h>

/* ---------- Peripheral base addresses ---------- */
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL
#define GPIOB_BASE      0x48000400UL
#define I2C1_BASE       0x40005400UL
#define USART2_BASE     0x40004400UL

/* ---------- Register structures ---------- */
#define RCC_AHB2ENR     (*(volatile uint32_t*)(RCC_BASE + 0x4C))
#define RCC_APB1ENR1    (*(volatile uint32_t*)(RCC_BASE + 0x58))
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE + 0x60))

#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t*)(GPIOA_BASE + 0x20))
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)(GPIOA_BASE + 0x08))
#define GPIOA_OTYPER    (*(volatile uint32_t*)(GPIOA_BASE + 0x04))
#define GPIOA_PUPDR     (*(volatile uint32_t*)(GPIOA_BASE + 0x0C))

#define GPIOB_MODER     (*(volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_AFRL      (*(volatile uint32_t*)(GPIOB_BASE + 0x20))
#define GPIOB_OTYPER    (*(volatile uint32_t*)(GPIOB_BASE + 0x04))
#define GPIOB_PUPDR     (*(volatile uint32_t*)(GPIOB_BASE + 0x0C))

/* I2C1 registers */
#define I2C1_CR1        (*(volatile uint32_t*)(I2C1_BASE + 0x00))
#define I2C1_TIMINGR    (*(volatile uint32_t*)(I2C1_BASE + 0x10))
#define I2C1_ISR        (*(volatile uint32_t*)(I2C1_BASE + 0x18))
#define I2C1_ICR        (*(volatile uint32_t*)(I2C1_BASE + 0x1C))
#define I2C1_TXDR       (*(volatile uint32_t*)(I2C1_BASE + 0x28))
#define I2C1_RXDR       (*(volatile uint32_t*)(I2C1_BASE + 0x24))
#define I2C1_CR2        (*(volatile uint32_t*)(I2C1_BASE + 0x04))

/* USART2 registers */
#define USART2_CR1      (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_BRR      (*(volatile uint32_t*)(USART2_BASE + 0x0C))
#define USART2_ISR      (*(volatile uint32_t*)(USART2_BASE + 0x1C))
#define USART2_TDR      (*(volatile uint32_t*)(USART2_BASE + 0x28))

/* ---------- BMI160 defines ---------- */
#define BMI160_ADDR     (0x69)
#define BMI160_WHOAMI   0x00
#define BMI160_CMD      0x7E
#define BMI160_ACC_DATA 0x12
#define BMI160_GYR_DATA 0x0C

/* ---------- Utility ---------- */
static void delay_ms(uint32_t ms) {
    for(uint32_t i=0; i<ms*4000; i++) __asm__("nop");
}

/* ---------- UART ---------- */
static void uart2_init(void) {
    RCC_AHB2ENR |= (1<<0);   // GPIOA clock
    RCC_APB1ENR1 |= (1<<17); // USART2 clock

    /* PA2: AF7 */
    GPIOA_MODER &= ~(3<<(2*2));
    GPIOA_MODER |=  (2<<(2*2));
    GPIOA_AFRL  |=  (7<<(4*2));

    USART2_BRR = 4000000/115200; // 80MHz clk
    USART2_CR1 = (1<<3)|(1<<0);   // TE + UE
}

static void uart2_write(char c) {
    while(!(USART2_ISR & (1<<7))); // TXE
    USART2_TDR = c;
}

static void uart2_print(char *s) {
    while(*s) uart2_write(*s++);
}

/* ---------- I2C ---------- */
static void i2c1_init(void) {
    RCC_AHB2ENR |= (1<<1);   // GPIOB clk
    RCC_APB1ENR1 |= (1<<21); // I2C1 clk

    // PB8 SCL, PB9 SDA (AF4)
    GPIOB_MODER &= ~((3<<(8*2)) | (3<<(9*2)));
    GPIOB_MODER |=  ((2<<(8*2)) | (2<<(9*2)));
    GPIOB_AFRL  |=  (4<<((8-8)*4)) | (4<<((9-8)*4));
    GPIOB_OTYPER |= (1<<8) | (1<<9);
    GPIOB_PUPDR  |= (1<<(8*2)) | (1<<(9*2));

    I2C1_CR1 &= ~(1<<0);             // disable
    I2C1_TIMINGR = 0x20303E5D;       // 100kHz timing
    I2C1_CR1 |= (1<<0);              // enable
}

static void i2c1_write(uint8_t addr, uint8_t reg, uint8_t val) {
    I2C1_CR2 = (addr<<1) | (1<<16);  // NBYTES=1
    I2C1_CR2 |= (1<<13);             // START
    while(!(I2C1_ISR & (1<<1)));     // TXIS
    I2C1_TXDR = reg;
    while(!(I2C1_ISR & (1<<6)));     // TC

    I2C1_CR2 = (addr<<1) | (1<<16);  // NBYTES=1
    I2C1_CR2 |= (1<<13);             // START
    while(!(I2C1_ISR & (1<<1)));
    I2C1_TXDR = val;
    while(!(I2C1_ISR & (1<<6)));
    I2C1_CR2 |= (1<<14);             // STOP
}

static uint8_t i2c1_read(uint8_t addr, uint8_t reg) {
    uint8_t val;
    // Write register
    I2C1_CR2 = (addr<<1) | (1<<16);
    I2C1_CR2 |= (1<<13);
    while(!(I2C1_ISR & (1<<1)));
    I2C1_TXDR = reg;
    while(!(I2C1_ISR & (1<<6)));
    // Read one byte
    I2C1_CR2 = (addr<<1)|(1<<10)|(1<<16); // RD_WRN=1
    I2C1_CR2 |= (1<<13);
    while(!(I2C1_ISR & (1<<2)));
    val = I2C1_RXDR;
    I2C1_CR2 |= (1<<14);
    return val;
}

/* ---------- MAIN ---------- */
int main(void) {
    uart2_init();
    i2c1_init();
    delay_ms(50);

    uart2_print("\r\nBMI160 Bare-Metal Init...\r\n");
    uint8_t id = i2c1_read(BMI160_ADDR, BMI160_WHOAMI);
    if(id == 0xD1)
        uart2_print("BMI160 detected!\r\n");
    else
        uart2_print("BMI160 not found!\r\n");

    /* Wake up sensors */
    i2c1_write(BMI160_ADDR, BMI160_CMD, 0x11); delay_ms(100);
    i2c1_write(BMI160_ADDR, BMI160_CMD, 0x15); delay_ms(100);

    char buf[64];
    while(1) {
        int16_t gx, gy, gz, ax, ay, az;

        gx = (int16_t)((i2c1_read(BMI160_ADDR, BMI160_GYR_DATA+1)<<8) |
                        i2c1_read(BMI160_ADDR, BMI160_GYR_DATA));
        gy = (int16_t)((i2c1_read(BMI160_ADDR, BMI160_GYR_DATA+3)<<8) |
                        i2c1_read(BMI160_ADDR, BMI160_GYR_DATA+2));
        gz = (int16_t)((i2c1_read(BMI160_ADDR, BMI160_GYR_DATA+5)<<8) |
                        i2c1_read(BMI160_ADDR, BMI160_GYR_DATA+4));

        ax = (int16_t)((i2c1_read(BMI160_ADDR, BMI160_ACC_DATA+1)<<8) |
                        i2c1_read(BMI160_ADDR, BMI160_ACC_DATA));
        ay = (int16_t)((i2c1_read(BMI160_ADDR, BMI160_ACC_DATA+3)<<8) |
                        i2c1_read(BMI160_ADDR, BMI160_ACC_DATA+2));
        az = (int16_t)((i2c1_read(BMI160_ADDR, BMI160_ACC_DATA+5)<<8) |
                        i2c1_read(BMI160_ADDR, BMI160_ACC_DATA+4));

        snprintf(buf, sizeof(buf),
                 "AX:%d AY:%d AZ:%d  GX:%d GY:%d GZ:%d\r\n",
                 ax, ay, az, gx, gy, gz);
        uart2_print(buf);
        delay_ms(300);
    }
}
