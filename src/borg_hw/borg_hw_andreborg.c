#include "../config.h"
#include "../makros.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "borg_hw.h"


#define COLDDR1  DDR(COLPORT1)
#define COLDDR2  DDR(COLPORT2)
#define ROWDDR   DDR(ROWPORT)

#if defined(__AVR_ATmega164__)   || \
    defined(__AVR_ATmega164P__)  || \
    defined(__AVR_ATmega324__)   || \
    defined(__AVR_ATmega324P__)  || \
    defined(__AVR_ATmega644__)   || \
    defined(__AVR_ATmega644P__)  || \
    defined(__AVR_ATmega1284__)  || \
    defined(__AVR_ATmega1284P__)
#	define TIMER0_OFF()        TIMSK0 &= ~(OCIE0A); TCCR0A = 0; TCCR0B = 0
#	define TIMER0_CTC_CS256()  TCCR0A = _BV(WGM01); TCCR0B = _BV(CS02)
#	define TIMER0_RESET()      TCNT0  = 0
#	define TIMER0_COMPARE(t)   OCR0A  = t
#	define TIMER0_INT_ENABLE() TIMSK0 |= _BV(OCIE0A)
#	define TIMER0_ISR          TIMER0_COMPA_vect
#else // ATmega16/32
#	define TIMER0_OFF()        TIMSK0 &= ~(OCIE0); TCCR0 = 0
#	define TIMER0_CTC_CS256()  TCCR0 = _BV(WGM01) | _BV(CS02)
#	define TIMER0_RESET()      TCNT0 = 0
#	define TIMER0_COMPARE(t)   OCR0  = t
#	define TIMER0_INT_ENABLE() TIMSK |= _BV(OCIE0)
#	define TIMER0_ISR          TIMER0_COMP_vect
#endif


// buffer which holds the currently shown frame
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];


// switch to next row
static void nextrow(uint8_t row) {
	//reset states of preceding row
	COLPORT1 = 0;
	COLPORT2 = 0;

	// short delay loop, to ensure proper deactivation of the drivers
	unsigned char i;
	for (i = 0; i < 10; i++) {
		asm volatile("nop");
	}

	if (row == 0) {
		// row 0: initialize first shift register
		ROWPORT &= ~(1 << PIN_MCLR);
		ROWPORT |= (1 << PIN_MCLR);
		ROWPORT |= (1 << PIN_DATA1);
		ROWPORT |= (1 << PIN_CLK);
		ROWPORT &= ~(1 << PIN_CLK);
		ROWPORT &= ~(1 << PIN_DATA1);
	} else if (row == 8) {
		// row 8: initialize second shift register
		ROWPORT &= ~(1 << PIN_MCLR);
		ROWPORT |= (1 << PIN_MCLR);
		ROWPORT |= (1 << PIN_DATA2);
		ROWPORT |= (1 << PIN_CLK);
		ROWPORT &= ~(1 << PIN_CLK);
		ROWPORT &= ~(1 << PIN_DATA2);
	} else {
		// remaining rows: just shift forward
		ROWPORT |= (1 << PIN_CLK);
		ROWPORT &= ~(1 << PIN_CLK);
	}

	// another delay loop, to ensure that the drivers are ready
	for (i = 0; i < 20; i++) {
		asm volatile("nop");
	}
}


// show a row
static void rowshow(unsigned char row, unsigned char plane) {
	// depending on the currently drawn plane, display the row for a specific
	// amount of time
	static unsigned char const ocr_table[] = {2, 5, 22};

	TIMER0_COMPARE(ocr_table[plane]);

	// output data of the current row to the column drivers
	COLPORT1 = pixmap[plane][row][0];
	COLPORT2 = pixmap[plane][row][1];
}


// interrupt handler
ISR(TIMER0_ISR) {
	static unsigned char plane = 0;
	static unsigned char row = 0;

	// increment both row and plane
	if (++plane == NUMPLANE) {
		plane = 0;
		if (++row == NUM_ROWS) {
			// reset watchdog
			wdt_reset();

			row = 0;
		}
		nextrow(row);
	}

	// output current row according to current plane
	rowshow(row, plane);
}


// disables timer, causing the watchdog to reset the MCU
void timer0_off() {
	cli();
	COLPORT1 = 0;
	COLPORT2 = 0;
	ROWPORT = 0;
	TIMER0_OFF();
	sei();
}


// initialize timer which triggers the interrupt
static void timer0_on() {
	TIMER0_CTC_CS256();  // CTC mode, prescaling conforms to clk/256
	TIMER0_RESET();      // set counter to 0
	TIMER0_COMPARE(20);  // compare with this value first
	TIMER0_INT_ENABLE(); // enable Timer/Counter0 Output Compare Match (A) Int.
}


void borg_hw_init() {
	// switch column ports to output mode
	COLDDR1 = 0xFF;
	COLDDR2 = 0xFF;

	// switch pins of the row port to output mode
	ROWDDR = (1<<PIN_MCLR) | (1<<PIN_CLK) | (1<<PIN_DATA1) | (1<<PIN_DATA2);

	// switch off all columns for now
	COLPORT1 = 0;
	COLPORT2 = 0;

	// reset shift registers for the rows
	ROWPORT = 0;

	timer0_on();

	// activate watchdog timer
	wdt_reset();
	wdt_enable(WDTO_15MS); // 15ms watchdog
}
