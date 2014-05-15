/**
 * @file borg_hw_lolshield.c
 * @brief Driver for Jimmie Rodgers' LoL Shield
 * @author Christian Kroll
 * @author Jimmie Rodgers
 * @date 2014
 * @copyright GNU Public License 2 or later
 * @see http://jimmieprodgers.com/kits/lolshield/
 *
 * This driver is partly based on Jimmie Rodger's LoL Shield Library which
 * is available at https://code.google.com/p/lolshield/ (parts of the file
 * "Charliplexing.cpp" have been incorporated into this file).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "../config.h"
#include "../makros.h"

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "borg_hw.h"

// buffer which holds the currently shown frame
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];

#define FRAMERATE 80UL
#define	CUTOFF(scaler)	((128 * 12 - 6) * FRAMERATE * scaler)

// BEWARE: only the Diavolino Kit is supported at the moment
// other platforms don't work, yet
const uint8_t
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega8__)
#   if F_CPU < CUTOFF(8)
        fastPrescaler = _BV(CS20),				// 1
        slowPrescaler = _BV(CS21);				// 8
#       define SLOWSCALERSHIFT 3
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(32)
        fastPrescaler = _BV(CS21),				// 8
        slowPrescaler = _BV(CS21) | _BV(CS20);			// 32
#       define SLOWSCALERSHIFT 5
#       define FASTSCALERSHIFT 2
#   elif F_CPU < CUTOFF(64)
        fastPrescaler = _BV(CS21),				// 8
        slowPrescaler = _BV(CS22);				// 64
#       define SLOWSCALERSHIFT 6
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(128)
        fastPrescaler = _BV(CS21) | _BV(CS20),			// 32
        slowPrescaler = _BV(CS22) | _BV(CS20);			// 128
#       define SLOWSCALERSHIFT 7
#       define FASTSCALERSHIFT 2
#   elif F_CPU < CUTOFF(256)
        fastPrescaler = _BV(CS21) | _BV(CS20),			// 32
        slowPrescaler = _BV(CS22) | _BV(CS21);			// 256
#       define SLOWSCALERSHIFT 8
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(1024)
        fastPrescaler = _BV(CS22) | _BV(CS20),			// 128
        slowPrescaler = _BV(CS22) | _BV(CS21) | _BV(CS20);	// 1024
#       define SLOWSCALERSHIFT 10
#       define FASTSCALERSHIFT 3
#   else
#       error frame rate is too low
#   endif
#else
#   error no support for this chip
#endif


ISR(TIMER2_OVF_vect) {
	// For each cycle, we have potential planes to display. Once every plane has
	// been displayed, then we move on to the next cycle.
	// NOTE: a "cycle" means a subset of LEDs

	// 12 Cycles of Matrix
	static uint8_t cycle = 0;

	// planes to display
	// NOTE: a "plane" in the Borgware is the same as a "page" in Jimmie's lib
	static uint8_t plane = 0;

	// pointer to corresponding bitmap
	uint8_t *p = &pixmap[plane][0][0];

	// tune timer values (fiddle around with them to adjust brightness)
	static uint8_t const prescaler[NUMPLANE + 1] = {3, 3, 3, 6};
	TCCR2B = prescaler[plane];
	static uint8_t const counts[NUMPLANE + 1] = {241, 198, 148, 255};
	TCNT2  = counts[plane];

	// Set sink pin to Vcc/source, turning off current.
	static uint8_t sink_b, sink_d = 0;
	PIND = sink_d;
	PINB = sink_b;

	// Set pins to input mode; Vcc/source become pullups.
	DDRD = 0;
	DDRB = 0;

	static uint8_t const sink_d_cycle[] =
		{0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static uint8_t const sink_b_cycle[] =
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20};

	uint8_t pins_d = sink_d = sink_d_cycle[cycle];
	uint8_t pins_b = sink_b = sink_b_cycle[cycle];

	// convert Borgware-2D framebuffer to LoL Shield cycles
	// (I could have done this with a lookup table, but that would be slower as
	// non-constant bit shifts are quite expensive on AVR)
    if (plane < NUMPLANE) {
		switch(cycle) {
		case 0:
			pins_b |= (0x02u & p[ 0]) << 4; // x=12, y= 0, high=13
			pins_b |= (0x02u & p[ 2]) << 3; // x=12, y= 1, high=12
			pins_b |= (0x02u & p[ 4]) << 2; // x=12, y= 2, high=11
			pins_b |= (0x02u & p[ 6]) << 1; // x=12, y= 3, high=10
			pins_b |= (0x02u & p[ 8]);      // x=12, y= 4, high= 9
			pins_b |= (0x02u & p[10]) >> 1; // x=12, y= 5, high= 8
			pins_d |= (0x02u & p[12]) << 6; // x=12, y= 6, high= 7
			pins_d |= (0x02u & p[14]) << 5; // x=12, y= 7, high= 6
			pins_d |= (0x02u & p[16]) << 4; // x=12, y= 8, high= 5
			break;
		case 1:
			pins_b |= (0x08u & p[ 0]) << 2; // x=10, y= 0, high=13
			pins_b |= (0x08u & p[ 2]) << 1; // x=10, y= 1, high=12
			pins_b |= (0x08u & p[ 4]);      // x=10, y= 2, high=11
			pins_b |= (0x08u & p[ 6]) >> 1; // x=10, y= 3, high=10
			pins_b |= (0x08u & p[ 8]) >> 2; // x=10, y= 4, high= 9
			pins_b |= (0x08u & p[10]) >> 3; // x=10, y= 5, high= 8
			pins_d |= (0x08u & p[12]) << 4; // x=10, y= 6, high= 7
			pins_d |= (0x08u & p[14]) << 3; // x=10, y= 7, high= 6
			pins_d |= (0x08u & p[16]) << 2; // x=10, y= 8, high= 5
			break;
		case 2:
			pins_b |= (0x20u & p[ 0]);      // x= 8, y= 0, high=13
			pins_b |= (0x20u & p[ 2]) >> 1; // x= 8, y= 1, high=12
			pins_b |= (0x20u & p[ 4]) >> 2; // x= 8, y= 2, high=11
			pins_b |= (0x20u & p[ 6]) >> 3; // x= 8, y= 3, high=10
			pins_b |= (0x20u & p[ 8]) >> 4; // x= 8, y= 4, high= 9
			pins_b |= (0x20u & p[10]) >> 5; // x= 8, y= 5, high= 8
			pins_d |= (0x20u & p[12]) << 2; // x= 8, y= 6, high= 7
			pins_d |= (0x20u & p[14]) << 1; // x= 8, y= 7, high= 6
			pins_d |= (0x20u & p[16]);      // x= 8, y= 8, high= 5
			break;
		case 3:
			pins_b |= (0x20u & p[ 1]);      // x= 0, y= 0, high=13
			pins_b |= (0x20u & p[ 3]) >> 1; // x= 0, y= 1, high=12
			pins_b |= (0x20u & p[ 5]) >> 2; // x= 0, y= 2, high=11
			pins_b |= (0x20u & p[ 7]) >> 3; // x= 0, y= 3, high=10
			pins_b |= (0x20u & p[ 9]) >> 4; // x= 0, y= 4, high= 9
			pins_b |= (0x20u & p[11]) >> 5; // x= 0, y= 5, high= 8
			pins_d |= (0x01u & p[16]) << 2; // x=13, y= 8, high= 2
			pins_d |= (0x04u & p[16]) << 1; // x=11, y= 8, high= 3
			pins_d |= (0x10u & p[16]);      // x= 9, y= 8, high= 4
			pins_d |= (0x20u & p[13]) << 2; // x= 0, y= 6, high= 7
			pins_d |= (0x20u & p[15]) << 1; // x= 0, y= 7, high= 6
			break;
		case 4:
			pins_b |= (0x10u & p[ 1]) << 1; // x= 1, y= 0, high=13
			pins_b |= (0x10u & p[ 3]);      // x= 1, y= 1, high=12
			pins_b |= (0x10u & p[ 5]) >> 1; // x= 1, y= 2, high=11
			pins_b |= (0x10u & p[ 7]) >> 2; // x= 1, y= 3, high=10
			pins_b |= (0x10u & p[ 9]) >> 3; // x= 1, y= 4, high= 9
			pins_b |= (0x10u & p[11]) >> 4; // x= 1, y= 5, high= 8
			pins_d |= (0x01u & p[14]) << 2; // x=13, y= 7, high= 2
			pins_d |= (0x04u & p[14]) << 1; // x=11, y= 7, high= 3
			pins_d |= (0x10u & p[13]) << 3; // x= 1, y= 6, high= 7
			pins_d |= (0x10u & p[14]);      // x= 9, y= 7, high= 4
			pins_d |= (0x20u & p[17]);      // x= 0, y= 8, high= 5
			break;
		case 5:
			pins_b |= (0x08u & p[ 1]) << 2; // x= 2, y= 0, high=13
			pins_b |= (0x08u & p[ 3]) << 1; // x= 2, y= 1, high=12
			pins_b |= (0x08u & p[ 5]);      // x= 2, y= 2, high=11
			pins_b |= (0x08u & p[ 7]) >> 1; // x= 2, y= 3, high=10
			pins_b |= (0x08u & p[ 9]) >> 2; // x= 2, y= 4, high= 9
			pins_b |= (0x08u & p[11]) >> 3; // x= 2, y= 5, high= 8
			pins_d |= (0x01u & p[12]) << 2; // x=13, y= 6, high= 2
			pins_d |= (0x04u & p[12]) << 1; // x=11, y= 6, high= 3
			pins_d |= (0x10u & p[12]);      // x= 9, y= 6, high= 4
			pins_d |= (0x10u & p[15]) << 2; // x= 1, y= 7, high= 6
			pins_d |= (0x10u & p[17]) << 1; // x= 1, y= 8, high= 5
			break;
		case 6:
			pins_b |= (0x04u & p[ 1]) << 3; // x= 3, y= 0, high=13
			pins_b |= (0x04u & p[ 3]) << 2; // x= 3, y= 1, high=12
			pins_b |= (0x04u & p[ 5]) << 1; // x= 3, y= 2, high=11
			pins_b |= (0x04u & p[ 7]);      // x= 3, y= 3, high=10
			pins_b |= (0x04u & p[ 9]) >> 1; // x= 3, y= 4, high= 9
			pins_d |= (0x01u & p[10]) << 2; // x=13, y= 5, high= 2
			pins_d |= (0x04u & p[10]) << 1; // x=11, y= 5, high= 3
			pins_d |= (0x08u & p[13]) << 4; // x= 2, y= 6, high= 7
			pins_d |= (0x08u & p[15]) << 3; // x= 2, y= 7, high= 6
			pins_d |= (0x08u & p[17]) << 2; // x= 2, y= 8, high= 5
			pins_d |= (0x10u & p[10]);      // x= 9, y= 5, high= 4
			break;
		case 7:
			pins_b |= (0x02u & p[ 1]) << 4; // x= 4, y= 0, high=13
			pins_b |= (0x02u & p[ 3]) << 3; // x= 4, y= 1, high=12
			pins_b |= (0x02u & p[ 5]) << 2; // x= 4, y= 2, high=11
			pins_b |= (0x02u & p[ 7]) << 1; // x= 4, y= 3, high=10
			pins_b |= (0x04u & p[11]) >> 2; // x= 3, y= 5, high= 8
			pins_d |= (0x01u & p[ 8]) << 2; // x=13, y= 4, high= 2
			pins_d |= (0x04u & p[ 8]) << 1; // x=11, y= 4, high= 3
			pins_d |= (0x04u & p[13]) << 5; // x= 3, y= 6, high= 7
			pins_d |= (0x04u & p[15]) << 4; // x= 3, y= 7, high= 6
			pins_d |= (0x04u & p[17]) << 3; // x= 3, y= 8, high= 5
			pins_d |= (0x10u & p[ 8]);      // x= 9, y= 4, high= 4
			break;
		case 8:
			pins_b |= (0x01u & p[ 1]) << 5; // x= 5, y= 0, high=13
			pins_b |= (0x01u & p[ 3]) << 4; // x= 5, y= 1, high=12
			pins_b |= (0x01u & p[ 5]) << 3; // x= 5, y= 2, high=11
			pins_b |= (0x02u & p[ 9]);      // x= 4, y= 4, high= 9
			pins_b |= (0x02u & p[11]) >> 1; // x= 4, y= 5, high= 8
			pins_d |= (0x01u & p[ 6]) << 2; // x=13, y= 3, high= 2
			pins_d |= (0x02u & p[13]) << 6; // x= 4, y= 6, high= 7
			pins_d |= (0x02u & p[15]) << 5; // x= 4, y= 7, high= 6
			pins_d |= (0x02u & p[17]) << 4; // x= 4, y= 8, high= 5
			pins_d |= (0x04u & p[ 6]) << 1; // x=11, y= 3, high= 3
			pins_d |= (0x10u & p[ 6]);      // x= 9, y= 3, high= 4
			break;
		case 9:
			pins_b |= (0x01u & p[ 7]) << 2; // x= 5, y= 3, high=10
			pins_b |= (0x01u & p[ 9]) << 1; // x= 5, y= 4, high= 9
			pins_b |= (0x01u & p[11]);      // x= 5, y= 5, high= 8
			pins_b |= (0x80u & p[ 0]) >> 2; // x= 6, y= 0, high=13
			pins_b |= (0x80u & p[ 2]) >> 3; // x= 6, y= 1, high=12
			pins_d |= (0x01u & p[ 4]) << 2; // x=13, y= 2, high= 2
			pins_d |= (0x01u & p[13]) << 7; // x= 5, y= 6, high= 7
			pins_d |= (0x01u & p[15]) << 6; // x= 5, y= 7, high= 6
			pins_d |= (0x01u & p[17]) << 5; // x= 5, y= 8, high= 5
			pins_d |= (0x04u & p[ 4]) << 1; // x=11, y= 2, high= 3
			pins_d |= (0x10u & p[ 4]);      // x= 9, y= 2, high= 4
			break;
		case 10:
			pins_b |= (0x40u & p[ 0]) >> 1; // x= 7, y= 0, high=13
			pins_b |= (0x80u & p[ 4]) >> 4; // x= 6, y= 2, high=11
			pins_b |= (0x80u & p[ 6]) >> 5; // x= 6, y= 3, high=10
			pins_b |= (0x80u & p[ 8]) >> 6; // x= 6, y= 4, high= 9
			pins_b |= (0x80u & p[10]) >> 7; // x= 6, y= 5, high= 8
			pins_d |= (0x01u & p[ 2]) << 2; // x=13, y= 1, high= 2
			pins_d |= (0x04u & p[ 2]) << 1; // x=11, y= 1, high= 3
			pins_d |= (0x10u & p[ 2]);      // x= 9, y= 1, high= 4
			pins_d |= (0x80u & p[12]);      // x= 6, y= 6, high= 7
			pins_d |= (0x80u & p[14]) >> 1; // x= 6, y= 7, high= 6
			pins_d |= (0x80u & p[16]) >> 2; // x= 6, y= 8, high= 5
			break;
		case 11:
			pins_b |= (0x40u & p[ 2]) >> 2; // x= 7, y= 1, high=12
			pins_b |= (0x40u & p[ 4]) >> 3; // x= 7, y= 2, high=11
			pins_b |= (0x40u & p[ 6]) >> 4; // x= 7, y= 3, high=10
			pins_b |= (0x40u & p[ 8]) >> 5; // x= 7, y= 4, high= 9
			pins_b |= (0x40u & p[10]) >> 6; // x= 7, y= 5, high= 8
			pins_d |= (0x01u & p[ 0]) << 2; // x=13, y= 0, high= 2
			pins_d |= (0x04u & p[ 0]) << 1; // x=11, y= 0, high= 3
			pins_d |= (0x10u & p[ 0]);      // x= 9, y= 0, high= 4
			pins_d |= (0x40u & p[12]) << 1; // x= 7, y= 6, high= 7
			pins_d |= (0x40u & p[14]);      // x= 7, y= 7, high= 6
			pins_d |= (0x40u & p[16]) >> 1; // x= 7, y= 8, high= 5
			break;
		}
    }

	// Enable pullups on new output pins.
	PORTD = pins_d;
	PORTB = pins_b;
	// Set pins to output mode; pullups become Vcc/source.
	DDRD = pins_d;
	DDRB = pins_b;
	// Set sink pin to GND/sink, turning on current.
	PIND = sink_d;
	PINB = sink_b;

	plane++;

	if (plane >= (NUMPLANE + 1)) {
		plane = 0;
		cycle++;
		if (cycle >= 12) {
			cycle = 0;
		}
	}
	wdt_reset();
}

void borg_hw_init() {
	// prepare Timer2
	TIMSK2 &= ~(_BV(TOIE2) | _BV(OCIE2A));
	TCCR2A &= ~(_BV(WGM21) | _BV(WGM20));
	TCCR2B &= ~_BV(WGM22);
	ASSR &= ~_BV(AS2);

    // Then start the display
    TIMSK2 |= _BV(TOIE2);
    TCCR2B = fastPrescaler;

    // interrupt ASAP
    TCNT2 = 0xff;

	// activate watchdog timer
	wdt_reset();
	wdt_enable(0x00); // 17ms watchdog
}
