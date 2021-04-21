#include "../platform.h"

void sleep_before(int ms) {
	// initialize timer
#if defined (__AVR_ATmega48__)    || \
    defined (__AVR_ATmega48P__)   || \
    defined (__AVR_ATmega88__)    || \
    defined (__AVR_ATmega88P__)   || \
    defined (__AVR_ATmega168__)   || \
    defined (__AVR_ATmega168P__)  || \
    defined (__AVR_ATmega328__)   || \
    defined (__AVR_ATmega328P__)  || \
    defined (__AVR_ATmega164__)   || \
    defined (__AVR_ATmega164P__)  || \
    defined (__AVR_ATmega324__)   || \
    defined (__AVR_ATmega324P__)  || \
    defined (__AVR_ATmega644__)   || \
    defined (__AVR_ATmega644P__)  || \
    defined (__AVR_ATmega1284__)  || \
    defined (__AVR_ATmega1284P__) || \
    defined (__AVR_ATmega32U4__)  || \
    defined (__AVR_ATmega1280__)  || \
    defined (__AVR_ATmega2560__)

#	ifndef USER_TIMER0_FOR_WAIT
	/* Timer1 for the masses */
	TCCR1B = _BV(WGM12) | _BV(CS12); //CTC Mode, clk/256
	OCR1A = (F_CPU/256000);	         //1000Hz
#	else
	/* Timer0
	 * Some Arduino/LoL Shield variants require Timer1 for multiplexing. Timer0,
	 * on the other hand, is free to use, which makes it a perfect candidate for
	 * our b2d_wait() function. */
	TCCR0A = _BV(WGM01);    // CTC mode
	TCCR0B = _BV(CS02);     // clk/256
	OCR0A = (F_CPU/256000);	//1000Hz
#	endif
#else
#	ifndef USER_TIMER0_FOR_WAIT
	/* Timer1 */
	TCCR1B = _BV(WGM12) | _BV(CS12); //CTC Mode, clk/256
	OCR1A = (F_CPU/256000);	         //1000Hz
#	else
	/* Timer0 */
	/* CTC mode, clk/256 */
	TCCR0 = _BV(WGM01) | _BV(CS02);
	OCR0 = (F_CPU/256000);	//1000Hz
#	endif
#endif
}

void sleep_step() {
    	// busy b2d_waiting for compare match interrupt flag
#if defined (__AVR_ATmega48__)    || \
    defined (__AVR_ATmega48P__)   || \
    defined (__AVR_ATmega88__)    || \
    defined (__AVR_ATmega88P__)   || \
    defined (__AVR_ATmega168__)   || \
    defined (__AVR_ATmega168P__)  || \
    defined (__AVR_ATmega328__)   || \
    defined (__AVR_ATmega328P__)  || \
    defined (__AVR_ATmega164__)   || \
    defined (__AVR_ATmega164P__)  || \
    defined (__AVR_ATmega324__)   || \
    defined (__AVR_ATmega324P__)  || \
    defined (__AVR_ATmega644__)   || \
    defined (__AVR_ATmega644P__)  || \
    defined (__AVR_ATmega1284__)  || \
    defined (__AVR_ATmega1284P__) || \
    defined (__AVR_ATmega32U4__)  || \
    defined (__AVR_ATmega1280__)  || \
    defined (__AVR_ATmega2560__)
#	ifndef USER_TIMER0_FOR_WAIT
		/* Timer1 for the masses */
		while(!(TIFR1 & _BV(OCF1A))); // b2d_wait for compare match flag
		TIFR1 |= _BV(OCF1A);          // reset that flag
#	else
		/* Timer0 for e.g. Arduino/LoL Shield */
		while(!(TIFR0 & _BV(OCF0A))); // b2d_wait for compare match flag
		TIFR0 |= _BV(OCF0A);          // reset that flag
#	endif
#else
#	ifndef USER_TIMER0_FOR_WAIT
		/* Timer1 for the masses */
		while (!(TIFR & _BV(OCF1A)));  // b2d_wait for compare match flag
		TIFR |= _BV(OCF1A);           // reset that flag
#	elif !defined(__AVR_ATmega8__)
		/* Timer0 */
		while(!(TIFR & _BV(OCF0)));   // b2d_wait for compare match flag
		TIFR |= _BV(OCF0);            // reset that flag
#	else
#		error Timer0 for b2d_wait() is not supported on ATmega8
#	endif
#endif
}