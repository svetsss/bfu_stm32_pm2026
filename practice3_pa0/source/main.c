#include <stdint.h>
#include "stm32f10x.h"

/* Кнопка A подключена к PA0; встроенный светодиод Blue Pill — к PC13. */
static volatile uint32_t milliseconds;

void SysTick_Handler(void) { ++milliseconds; }

int main(void)
{
    static const uint16_t half_period_ms[] = {500, 250, 125, 1000};
    uint32_t last_toggle = 0, changed_at = 0;
    uint8_t speed = 0, raw_previous = 1, stable = 1;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;
    GPIOA->CRL = (GPIOA->CRL & ~0xFU) | 0x8U; /* PA0: вход с подтяжкой */
    GPIOA->ODR |= 1U;                           /* подтяжка к питанию */
    GPIOC->CRH = (GPIOC->CRH & ~(0xFU << 20)) | (0x2U << 20);
    GPIOC->BSRR = 1U << 13;                    /* светодиод выключен */

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000U);   /* прерывание каждую мс */

    for (;;) {
        uint32_t now = milliseconds;
        uint8_t raw = (GPIOA->IDR & 1U) != 0U;

        if (raw != raw_previous) {
            raw_previous = raw;
            changed_at = now;
        }
        if (raw != stable && (uint32_t)(now - changed_at) >= 30U) {
            stable = raw;
            if (!stable) {                     /* одно действие на нажатие */
                speed = (speed + 1U) % 4U;
                last_toggle = now;
            }
        }
        if ((uint32_t)(now - last_toggle) >= half_period_ms[speed]) {
            last_toggle = now;
            GPIOC->ODR ^= 1U << 13;
        }
    }
}
