#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include <usbdrv.h>

#include "../config.h"

uint8_t buffer;
uint16_t countdown;

void pgm_read_block(uint8_t *pTarget, const uint8_t *pSource, size_t len) {
	for(size_t i = 0; i < len; i++) {
		*pTarget++ = pgm_read_byte(pSource++);
	}
}

void usbReset() {
	DDRB |= (1 << PB0) | (1 << PB1);	/* enable USB pins as output */
	_delay_ms(15);				/* sleeping for 15ms */
	DDRB &= ~(1 << PB0) & ~(1 << PB1);	/* disbale USB pins as output */
}

void hardwareInit() {
	usbReset();

	/* Relay Ports */
	DDRB |= (1 << PB3) | (1 << PB5);
	PORTB &= ~(1 << PB3) & ~(1 << PB5);

	/* LEDs */
	DDRC |= (1 << PC0) | (1 << PC1);	/* set LED pins as output */
	PORTC |= (1 << PC0) | (1 << PC1);	/* clear LEDs (active low) */

	/* Countdown Timer */
	TIMSK = (1 << OCIE1A);
	OCR1A = 12000;
}

ISR(TIMER1_COMPA_vect) {
	countdown--;

	if (!countdown) {
		PORTC |= (1 << PC0) | (1 << PC1);
		PORTB &= ~(1 << PB3) & ~(1 << PB5);
		TCCR1B = 0;			/* stop timer */
	}
}

void delay_1ms(uint16_t ms) {
	uint16_t i;

	for(i=0; i<ms ;i++)
		_delay_ms(1);
}

uint8_t usbFunctionSetup(uint8_t data[8]) {
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR) {
		switch (rq->bRequest) {
			case USBRQ_VENDOR_PULSE: {
				char port = 0;
				if (rq->wValue.bytes[0] & 0x01) {
					port |= (1 << PB3);
					PORTC &= ~(1 << PC0);
				}
				if (rq->wValue.bytes[0] & 0x02) {
					port |= (1 << PB5);
					PORTC &= ~(1 << PC1);
				}

				PORTB |= port;

				countdown = rq->wIndex.word;
				TCCR1B = (1 << CS10) | (1 << WGM12); /* start countdown timer */

				buffer = 0;
				return 1;
			}

			case USBRQ_VENDOR_SET: {
				char port = 0;
				PORTC |= (1 << PC0) | (1 << PC1);

				if (rq->wValue.bytes[0] & 0x01) {
					port |= (1 << PB3);
					PORTC &= ~(1 << PC0);
				}
				if (rq->wValue.bytes[0] & 0x02) {
					port |= (1 << PB5);
					PORTC &= ~(1 << PC1);
				}

				PORTB = port | (PORTB & ~((1 << PB3) | (1 << PB5)));

				buffer = 0;
				return 1;
			}

			case USBRQ_VENDOR_GET: {
				buffer = 0;
				if (PINB & (1 << PB3))
					buffer |= 0x01;
				if (PINB & (1 << PB5))
					buffer |= 0x02;
				if (PINB & (1 << PB2))
					buffer |= 0x04;
				if (PINB & (1 << PB4))
					buffer |= 0x08;

				return 1;
			}
		}
	}

	return 0;
}

int main(void) {
	wdt_enable(WDTO_2S);	/* enable watchdog timer 2s */

	usbInit();		/* initialize USB stack processing */
	hardwareInit();		/* setup io & starts timer */
	sei();			/* enable global interrupts */

	usbMsgPtr = &buffer;

	/* main loop */
	for (;;) {
		wdt_reset();	/* reset the watchdog */
		usbPoll();	/* poll the USB stack */
	}

	return 0;
}

