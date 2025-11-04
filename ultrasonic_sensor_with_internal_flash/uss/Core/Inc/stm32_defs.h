#ifndef STM32_DEFS_H
#define STM32_DEFS_H
#include <stdint.h>

/* Base addresses */
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL
#define GPIOB_BASE      0x48000400UL
#define GPIOC_BASE      0x48000800UL
#define USART2_BASE     0x40004400UL
#define SPI1_BASE       0x40013000UL

/* RCC registers */
#define RCC             ((RCC_TypeDef *) RCC_BASE)
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t ICSCR;
    volatile uint32_t CFGR;
    uint32_t RESERVED0;
    volatile uint32_t PLLCFGR;
    volatile uint32_t PLLSAI1CFGR;
    volatile uint32_t CIER;
    volatile uint32_t CIFR;
    volatile uint32_t CICR;
    uint32_t RESERVED1;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t RESERVED2;
    volatile uint32_t APB1RSTR1;
    volatile uint32_t APB1RSTR2;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED3;
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t RESERVED4;
    volatile uint32_t APB1ENR1;
    volatile uint32_t APB1ENR2;
    volatile uint32_t APB2ENR;
} RCC_TypeDef;

/* GPIO structure */
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
    volatile uint32_t BRR;
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC ((GPIO_TypeDef *) GPIOC_BASE)

/* USART */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
} USART_TypeDef;
#define USART2 ((USART_TypeDef *) USART2_BASE)

/* SPI */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t CRCPR;
    volatile uint32_t RXCRCR;
    volatile uint32_t TXCRCR;
    volatile uint32_t I2SCFGR;
    volatile uint32_t I2SPR;
} SPI_TypeDef;
#define SPI1 ((SPI_TypeDef *) SPI1_BASE)

/* RCC bits */
#define RCC_CR_HSION        (1U<<8)
#define RCC_CR_HSIRDY       (1U<<10)
#define RCC_CFGR_SW_HSI     (0x1U)
#define RCC_CFGR_SWS_HSI    (0x1U<<2)
#define RCC_AHB2ENR_GPIOAEN (1U<<0)
#define RCC_AHB2ENR_GPIOBEN (1U<<1)
#define RCC_AHB2ENR_GPIOCEN (1U<<2)
#define RCC_APB1ENR1_USART2EN (1U<<17)
#define RCC_APB2ENR_SPI1EN  (1U<<12)

/* Simple delay (blocking) */
static inline void delay_ms(uint32_t ms) {
    for(uint32_t i=0;i<ms*4000;i++) __asm volatile("nop");
}
static inline void delay_us(uint32_t us) {
    for(uint32_t i=0;i<us*4;i++) __asm volatile("nop");
}

#endif
