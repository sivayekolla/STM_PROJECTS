#include "stm32l476xx.h"

int main(void)
{
    // ===== Enable GPIOA clock =====
    RCC->AHB2ENR |= (1U << 0);             // Bit0 -> GPIOA clock enable

    // ===== Set PA5 as output =====
    GPIOA->MODER &= ~(3U << (2 * 5));      // Clear mode bits for pin 5
    GPIOA->MODER |=  (1U << (2 * 5));      // Set pin 5 as output mode (01)

    // ===== Infinite toggle loop =====
    while (1)
    {
        // Toggle PA5 (LED)
        GPIOA->ODR ^= (1U << 5);           // XOR toggles the bit

        // Simple delay loop
        for (volatile int i = 0; i < 500000; i++);
    }
}
