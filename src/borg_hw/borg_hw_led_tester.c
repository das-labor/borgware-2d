#include "../config.h"
#include "../makros.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "borg_hw.h"


#if defined(__AVR_ATmega48__)   || \
    defined(__AVR_ATmega48P__)  || \
    defined(__AVR_ATmega88__)   || \
    defined(__AVR_ATmega88P__)  || \
    defined(__AVR_ATmega168__)  || \
    defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328__)  || \
    defined(__AVR_ATmega328P__)
#	define TIMER0_OFF()        TCCR0A = 0; TCCR0B = 0
#	define TIMER0_MODE_CS256() TCCR0A = _BV(WGM01); TCCR0B = _BV(CS02)
#	define TIMER0_RESET()      TCNT0  = 0
#	define TIMER0_COMPARE(t)   OCR0A  = t
#	define TIMER0_INT_ENABLE() TIMSK0 = _BV(OCIE0A)
#	define TIMER0_ISR          TIMER0_COMPA_vect
#else // ATmega8
#	define TIMER0_OFF()        TCCR0 = 0
#	define TIMER0_MODE_CS256() TCCR0 = _BV(CS02)
#	define TIMER0_RESET()      TCNT0 = 0
#	define TIMER0_COMPARE(t)   TCNT0 = (0xff - t)
#	define TIMER0_INT_ENABLE() TIMSK = _BV(TOIE0)
#	define TIMER0_ISR          TIMER0_OVF_vect
#endif


/* Output data of the current row to the column drivers:
 * Column:  15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
 * Pin:    PD2 PD3 PC5 PC4 PC3 PC2 PC1 PC0 PD7 PD6 PB5 PB4 PB3 PB2 PB1 PB0
 */
#define DISPLAY_PINS_B (_BV(PB5)|_BV(PB4)|_BV(PB3)|_BV(PB2)|_BV(PB1)|_BV(PB0))
#define DISPLAY_PINS_C (_BV(PC5)|_BV(PC4)|_BV(PC3)|_BV(PC2)|_BV(PC1)|_BV(PC0))
#define DISPLAY_PINS_D (_BV(PD7)|_BV(PD6)|_BV(PD3)|_BV(PD2))

#define ROW_DUMMY_PORT PORTB
#define ROW_DUMMY_DDR DDRB
#define ROW_DUMMY_PIN PB7
#define ROW_LED_PORT PORTD
#define ROW_LED_DDR DDRD
#define ROW_LED_PIN PD5

// buffer which holds the currently shown frame
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];


// switch to next row
static void nextrow(uint8_t row) {
	// switch off all columns for now
	PORTB &= ~(DISPLAY_PINS_B);
	PORTC &= ~(DISPLAY_PINS_C);
	PORTD &= ~(DISPLAY_PINS_D);

	// short delay loop, to ensure proper deactivation of the drivers
	unsigned char i;
	for (i = 0; i < 10; i++) {
		asm volatile("nop");
	}

	if (row == LED_TESTER_DISPLAYED_ROW) {
		// configured row: unblock LED MOSFET, block resistor MOSFET
		ROW_DUMMY_PORT &= ~_BV(ROW_DUMMY_PIN); // block resistor MOSFET
		ROW_LED_PORT   |= _BV(ROW_LED_PIN);    // unblock LED MOSFET
	} else { /* fake rows simulated by resistors */
		// remaining rows: unblock resistor MOSFET, block LED MOSFET
		ROW_LED_PORT   &= ~_BV(ROW_LED_PIN);   // block LED MOSFET
		ROW_DUMMY_PORT |= _BV(ROW_DUMMY_PIN);  // block resistor MOSFET
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
#ifdef HIGH_CONTRAST
	static unsigned char const ocr_table[] = {2, 5, 22};
#else
	static unsigned char const ocr_table[] = {3, 4, 22};
#endif

	TIMER0_COMPARE(ocr_table[plane]);

	uint8_t port_b = (PORTB & ~DISPLAY_PINS_B) | (0x3f & pixmap[plane][row][0]);
	uint8_t port_c = (PORTC & ~DISPLAY_PINS_C) | (0x3f & pixmap[plane][row][1]);
	uint8_t port_d = (PORTD & ~DISPLAY_PINS_D) | (0xc0 & pixmap[plane][row][0]);
	if (0x40u & pixmap[plane][row][1]) port_d |= _BV(PD3);
	if (0x80u & pixmap[plane][row][1]) port_d |= _BV(PD2);

	PORTB = port_b;
	PORTC = port_c;
	PORTD = port_d;
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
	// switch off all columns
	PORTB &= ~(DISPLAY_PINS_B);
	PORTC &= ~(DISPLAY_PINS_C);
	PORTD &= ~(DISPLAY_PINS_D);
	TIMER0_OFF();
	sei();
}


// initialize timer which triggers the interrupt
static void timer0_on() {
	TIMER0_MODE_CS256(); // CTC mode, prescaling conforms to clk/256
	TIMER0_RESET();      // set counter to 0
	TIMER0_COMPARE(20);  // compare with this value first
	TIMER0_INT_ENABLE(); // enable Timer/Counter0 Output Compare Match (A) Int.
}


void borg_hw_init() {
	// switch column and row ports to output mode
	DDRB |= DISPLAY_PINS_B;
	DDRC |= DISPLAY_PINS_C;
	DDRD |= DISPLAY_PINS_D;
	ROW_DUMMY_DDR |= _BV(ROW_DUMMY_PIN);
	ROW_LED_DDR |= _BV(ROW_LED_PIN);

	// switch off all columns for now
	PORTB &= ~(DISPLAY_PINS_B);
	PORTC &= ~(DISPLAY_PINS_C);
	PORTD &= ~(DISPLAY_PINS_D);

	// switch off all rows
	ROW_DUMMY_PORT &= ~_BV(ROW_DUMMY_PIN);
	ROW_LED_PORT &= ~_BV(ROW_LED_PIN);

	timer0_on();

	// activate watchdog timer
	wdt_reset();
	wdt_enable(WDTO_15MS); // 15ms watchdog
}
