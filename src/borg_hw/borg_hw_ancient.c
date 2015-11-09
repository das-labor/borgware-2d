#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "../config.h"
#include "../makros.h"
#include "borg_hw.h"


// define row port
#ifndef ROWPORT
#	define ROWPORT PORTA
#endif
#define ROWDDR DDR(ROWPORT)


// define column port
#ifndef COLPORT
#	define COLPORT PORTC
#endif
#define COLDDR DDR(COLPORT)


// Timer0 control
#if defined(__AVR_ATmega164__)   || \
    defined(__AVR_ATmega164P__)  || \
    defined(__AVR_ATmega324__)   || \
    defined(__AVR_ATmega324P__)  || \
    defined(__AVR_ATmega644__)   || \
    defined(__AVR_ATmega644P__)  || \
    defined(__AVR_ATmega1284__)  || \
    defined(__AVR_ATmega1284P__)
#	define TIMER0_OFF()        TIMSK0 &= ~(OCIE0A); TCCR0A = 0; TCCR0B = 0
#	define TIMER0_CTC_CS64()   TCCR0A = _BV(WGM01); TCCR0B = _BV(CS01) | _BV(CS00)
#	define TIMER0_RESET()      TCNT0  = 0
#	define TIMER0_COMPARE(t)   OCR0A  = t
#	define TIMER0_INT_ENABLE() TIMSK0 |= _BV(OCIE0A)
#	define TIMER0_ISR          TIMER0_COMPA_vect
#elif defined(__AVR_ATmega16__) || \
      defined(__AVR_ATmega32__)
#	define TIMER0_OFF()        TIMSK &= ~(OCIE0); TCCR0 = 0
#	define TIMER0_CTC_CS64()   TCCR0 = _BV(WGM01) | _BV(CS01) | _BV(CS00)
#	define TIMER0_RESET()      TCNT0 = 0
#	define TIMER0_COMPARE(t)   OCR0  = t
#	define TIMER0_INT_ENABLE() TIMSK |= _BV(OCIE0)
#	define TIMER0_ISR          TIMER0_COMP_vect
#else
#   error MCU not supported
#endif


// buffer which holds the currently shown frame
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];


ISR(TIMER0_ISR)
{
	static unsigned char plane = 0;
	static unsigned char row = 0;

	ROWPORT = 0;
	row++;
	if ((COLPORT <<= 1) == 0) {
		row = 0;
		COLPORT = 1;

		if (++plane == NUMPLANE)
		{
			plane = 0;

#ifdef WATCHDOG_ENABLED
			// reset watchdog
			wdt_reset();
#endif
		}
	}

	for (unsigned char x = 0; x < 10; x++) {
		asm volatile ("nop");
	}

	ROWPORT = pixmap[plane][row][0];
}


void timer0_off(){
	cli();

	COLPORT = 0;
	ROWPORT = 0;

	TIMER0_OFF();

	sei();
}


// initialize timer which triggers the interrupt
static void timer0_on() {
	TIMER0_CTC_CS64();    // CTC mode, prescaling conforms to clk/64
	TIMER0_RESET();       // set counter to 0
	TIMER0_COMPARE(0x10); // compare with this value first
	TIMER0_INT_ENABLE();  // enable Timer/Counter0 Output Compare Match (A) Int.
}


void borg_hw_init(){
	// switch all pins of both the row and the column port to output mode
	ROWDDR = 0xFF;
	COLDDR = 0xFF;

	// switch off all rows and columns for now
	COLPORT = 0;
	ROWPORT = 0;

	timer0_on();

#ifdef WATCHDOG_ENABLED
	// activate watchdog timer
	wdt_reset();
	wdt_enable(WDTO_15MS); // 15ms watchdog
#endif
}
