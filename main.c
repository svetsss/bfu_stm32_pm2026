#include <stdint.h>
#include <stm32f10x.h>

enum {
	BUTTON_PIN = 0,
	LED_PIN = 13,
	SAMPLE_DELAY_TICKS = 1000,
	SLOW_HALF_PERIOD_TICKS = 1000000,
	FAST_HALF_PERIOD_TICKS = 200000,
	DEBOUNCE_SAMPLES = 3
};

static void delay(uint32_t ticks) {
	for (uint32_t i = 0; i < ticks; ++i) {
		__NOP();
	}
}

static uint8_t button_is_pressed(void) {
	/* PB0 uses the internal pull-up, so a pressed button reads as zero. */
	return (GPIOB->IDR & (1U << BUTTON_PIN)) == 0U;
}

int __attribute((noreturn)) main(void) {
	uint8_t raw_button = 0U;
	uint8_t stable_button = 0U;
	uint8_t debounce_count = 0U;
	uint8_t fast_mode = 0U;
	uint32_t blink_ticks = 0U;
	uint32_t half_period_ticks = SLOW_HALF_PERIOD_TICKS;

	/* Enable AFIO, GPIOB (button) and GPIOC (on-board LED). */
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |
	                  RCC_APB2ENR_IOPBEN |
	                  RCC_APB2ENR_IOPCEN;

	/* PC13: general-purpose push-pull output, maximum speed 10 MHz. */
	GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
	GPIOC->CRH |= GPIO_CRH_MODE13_0;
	GPIOC->ODR |= (1U << LED_PIN); /* Blue Pill LED is active-low: start off. */

	/* PB0: input with internal pull-up. The button connects PB0 to GND. */
	GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
	GPIOB->CRL |= GPIO_CRL_CNF0_1;
	GPIOB->ODR |= (1U << BUTTON_PIN);

	while (1) {
		const uint8_t sample = button_is_pressed();

		/* Accept a new state only after several equal samples (debouncing). */
		if (sample == raw_button) {
			if (debounce_count < DEBOUNCE_SAMPLES) {
				++debounce_count;
			}
		} else {
			raw_button = sample;
			debounce_count = 0U;
		}

		if ((debounce_count == DEBOUNCE_SAMPLES) &&
		    (stable_button != raw_button)) {
			stable_button = raw_button;
			if (stable_button != 0U) {
				fast_mode ^= 1U;
				half_period_ticks = fast_mode != 0U
				                        ? FAST_HALF_PERIOD_TICKS
				                        : SLOW_HALF_PERIOD_TICKS;
			}
		}

		delay(SAMPLE_DELAY_TICKS);
		blink_ticks += SAMPLE_DELAY_TICKS;

		if (blink_ticks >= half_period_ticks) {
			GPIOC->ODR ^= (1U << LED_PIN);
			blink_ticks = 0U;
		}
	}
}
