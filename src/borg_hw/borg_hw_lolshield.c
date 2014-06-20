/**
 * @file borg_hw_lolshield.c
 * @brief Driver for Jimmie Rodgers' LoL Shield
 * @author Christian Kroll <chris@das-labor.org>
 * @date 2014
 * @copyright GNU Public License 2 or later
 * @see http://jimmieprodgers.com/kits/lolshield/
 *
 * This driver is partly based on the LoL Shield Library which is available at
 * https://code.google.com/p/lolshield/ (parts of the file "Charliplexing.cpp"
 * have been incorporated into this file).
 *
 * With contributions from (via "Charliplexing.cpp"):
 *  Alex Wenger <a.wenger@gmx.de> http://arduinobuch.wordpress.com/
 *  Matt Mets <mahto@cibomahto.com> http://cibomahto.com/
 *  Timer init code from MsTimer2 - Javier Valencia <javiervalencia80@gmail.com>
 *  Misc functions from Benjamin Sonntag <benjamin@sonntag.fr>
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
#if NUMPLANE >= 8
#	include <math.h>
#endif
#include "borg_hw.h"

// buffer which holds the currently shown frame
unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];


/* adjust frame rate at the menuconfig, this is just a fallback */
#ifndef FRAMERATE
#	define FRAMERATE 80
#elif FRAMERATE < 1
#	error FRAMERATE must be greater than 0
#endif

// Number of ticks of the prescaled timer per cycle per frame, based on the
// CPU clock speed and the desired frame rate.
#define	TICKS (F_CPU + 6ul * (FRAMERATE << SLOWSCALERSHIFT)) / \
	(12ul * (FRAMERATE << SLOWSCALERSHIFT))
#define	CUTOFF(scaler)	((128ul * 12 - 6) * FRAMERATE * scaler##ul)

#if defined (__AVR_ATmega8__)    || \
    defined (__AVR_ATmega48__)   || \
    defined (__AVR_ATmega48P__)  || \
    defined (__AVR_ATmega88__)   || \
    defined (__AVR_ATmega88P__)  || \
    defined (__AVR_ATmega168__)  || \
    defined (__AVR_ATmega168P__) || \
    defined (__AVR_ATmega328__)  || \
    defined (__AVR_ATmega328P__) || \
    defined (__AVR_ATmega1280__) || \
    defined (__AVR_ATmega2560__)
#	if F_CPU < CUTOFF(8)
#		define FASTPRESCALER (_BV(CS20))                          // 1
#		define SLOWPRESCALER (_BV(CS21))                          // 8
#		define FASTSCALERSHIFT 3
#		define SLOWSCALERSHIFT 3
#	elif F_CPU < CUTOFF(32)
#		define FASTPRESCALER (_BV(CS21))                          // 8
#		define SLOWPRESCALER (_BV(CS21) | _BV(CS20))              // 32
#		define FASTSCALERSHIFT 2
#		define SLOWSCALERSHIFT 5
#	elif F_CPU < CUTOFF(64)
#		define FASTPRESCALER (_BV(CS21))                          // 8
#		define SLOWPRESCALER (_BV(CS22))                          // 64
#		define FASTSCALERSHIFT 3
#		define SLOWSCALERSHIFT 6
#   elif F_CPU < CUTOFF(128)
#		define FASTPRESCALER (_BV(CS21) | _BV(CS20))              // 32
#		define SLOWPRESCALER (_BV(CS22) | _BV(CS20))              // 128
#		define FASTSCALERSHIFT 2
#		define SLOWSCALERSHIFT 7
#	elif F_CPU < CUTOFF(256)
#		define FASTPRESCALER (_BV(CS21) | _BV(CS20))              // 32
#		define SLOWPRESCALER (_BV(CS22) | _BV(CS21))              // 256
#		define FASTSCALERSHIFT 3
#		define SLOWSCALERSHIFT 8
#   elif F_CPU < CUTOFF(1024)
#		define FASTPRESCALER (_BV(CS22) | _BV(CS20))              // 128
#		define SLOWPRESCALER (_BV(CS22) | _BV(CS21) | _BV(CS20))  // 1024
#		define FASTSCALERSHIFT 3
#		define SLOWSCALERSHIFT 10
#	else
#		error frame rate is too low
#	endif
#elif defined (__AVR_ATmega32U4__)
#	if F_CPU < CUTOFF(8)
#		define FASTPRESCALER (_BV(WGM12) | _BV(CS10))             // 1
#		define SLOWPRESCALER (_BV(WGM12) | _BV(CS11))             // 8
#		define FASTSCALERSHIFT 3
#		define SLOWSCALERSHIFT 3
#	elif F_CPU < CUTOFF(64)
#		define FASTPRESCALER (_BV(WGM12) | _BV(CS11))             // 8
#		define SLOWPRESCALER (_BV(WGM12) | _BV(CS11) | _BV(CS10)) // 64
#		define FASTSCALERSHIFT 3
#		define SLOWSCALERSHIFT 6
#	elif F_CPU < CUTOFF(256)
#		define FASTPRESCALER (_BV(WGM12) | _BV(CS11) | _BV(CS10)) // 64
#		define SLOWPRESCALER (_BV(WGM12) | _BV(CS12))             // 256
#		define FASTSCALERSHIFT 2
#		define SLOWSCALERSHIFT 8
#	elif F_CPU < CUTOFF(1024)
#		define FASTPRESCALER (_BV(WGM12) | _BV(CS12))             // 256
#		define SLOWPRESCALER (_BV(WGM12) | _BV(CS12) | _BV(CS10)) // 1024
#		define FASTSCALERSHIFT 2
#		define SLOWSCALERSHIFT 10
#	else
#		error frame rate is too low
#	endif
#else
#   error no support for this chip
#endif

#if NUMPLANE < 8
uint8_t const prescaler[NUMPLANE + 1] = {
	FASTPRESCALER,
#	if NUMPLANE >= 2
	FASTPRESCALER,
#	endif
#	if NUMPLANE >= 3
	FASTPRESCALER,
#	endif
#	if NUMPLANE >= 4
	FASTPRESCALER,
#	endif
#	if NUMPLANE >= 5
	FASTPRESCALER,
#	endif
#	if NUMPLANE >= 6
	FASTPRESCALER,
#	endif
#	if NUMPLANE >= 7
	FASTPRESCALER,
#	endif
	SLOWPRESCALER
};
#else
uint8_t prescaler[NUMPLANE + 1] = {0};
#endif

/* adjust brightness in the menuconfig, this is just a fallback */
#ifndef BRIGHTNESS
#	define BRIGHTNESS 127 /* full brightness by default */
#elif BRIGHTNESS < 0 || BRIGHTNESS > 127
#	error BRIGHTNESS must be between 0 and 127
#endif

#define BRIGHTNESSPERCENT ((BRIGHTNESS * BRIGHTNESS + 8ul) / 16ul)
#define M (TICKS << FASTSCALERSHIFT) * BRIGHTNESSPERCENT /*10b*/
#define C(x) ((M * (unsigned long)(x * 1024UL) + (1UL << 19)) >> 20) /*10b+10b-20b=0b*/

#define COUNT(u, v) (256 - (((C(v) - C(u)) != 0) ? (C(v) - C(u)) : 1))
#define LAST_COUNT(u) (256 - (((TICKS - (C(u) >> FASTSCALERSHIFT)) != 0) ? \
		(TICKS - (C(u) >> FASTSCALERSHIFT)) : 1))

#if NUMPLANE < 8
// NOTE: The argumentS of COUNT() are calculated as follows:
// pow((double)x / (double)NUMPLANE, 1.8) with 0 <= x <= NUMPLANE
// Changing the scale of 1.8 invalidates any tables above!
uint8_t const counts[NUMPLANE + 1] = {
#	if NUMPLANE == 1
		COUNT(0.000000000000000000000000, 1.000000000000000000000000),
#	elif NUMPLANE == 2
		COUNT(0.000000000000000000000000, 0.287174588749258719033719),
		COUNT(0.287174588749258719033719, 1.000000000000000000000000),
#	elif NUMPLANE == 3
		COUNT(0.000000000000000000000000, 0.138414548846168578011273),
		COUNT(0.138414548846168578011273, 0.481987453865643789008288),
		COUNT(0.481987453865643789008288, 1.000000000000000000000000),
#	elif NUMPLANE == 4
		COUNT(0.000000000000000000000000, 0.082469244423305887448095),
		COUNT(0.082469244423305887448095, 0.287174588749258719033719),
		COUNT(0.287174588749258719033719, 0.595813410589956848895099),
		COUNT(0.595813410589956848895099, 1.000000000000000000000000),
#	elif NUMPLANE == 5
		COUNT(0.000000000000000000000000, 0.055189186458448592775827),
		COUNT(0.055189186458448592775827, 0.192179909437029006191722),
		COUNT(0.192179909437029006191722, 0.398723883569384374148115),
		COUNT(0.398723883569384374148115, 0.669209313658414961523135),
		COUNT(0.669209313658414961523135, 1.000000000000000000000000),
#	elif NUMPLANE == 6
		COUNT(0.000000000000000000000000, 0.039749141141812646682574),
		COUNT(0.039749141141812646682574, 0.138414548846168578011273),
		COUNT(0.138414548846168578011273, 0.287174588749258719033719),
		COUNT(0.287174588749258719033719, 0.481987453865643789008288),
		COUNT(0.481987453865643789008288, 0.720234228706005730202833),
		COUNT(0.720234228706005730202833, 1.000000000000000000000000),
#	elif NUMPLANE == 7
		COUNT(0.000000000000000000000000, 0.030117819624378608378557),
		COUNT(0.030117819624378608378557, 0.104876339357015443964904),
		COUNT(0.104876339357015443964904, 0.217591430058779483625031),
		COUNT(0.217591430058779483625031, 0.365200625214741059210155),
		COUNT(0.365200625214741059210155, 0.545719579451565794947498),
		COUNT(0.545719579451565794947498, 0.757697368024318751444923),
		COUNT(0.757697368024318751444923, 1.000000000000000000000000),
#	endif
		LAST_COUNT(1.0)
};
#else
uint8_t counts[NUMPLANE + 1];
#endif


#if NUMPLANE >= 8
/**
 *  Set the overall brightness of the screen from 0 (very dim) to 127 (full on).
 */
static void setBrightness()
{
#	warning "NUMPLANE >= 8 links floating point stuff into the image"
	/*   ---- This needs review! Please review. -- thilo  */
	// set up page counts
	uint8_t i;
	// NOTE: Changing "scale" invalidates any tables above!
	const float scale = 1.8f;
	int temp_counts[NUMPLANE + 1] = {0};
	for (i = 1; i < (NUMPLANE + 1); i++) {
		temp_counts[i] = C(pow(i / (float)(NUMPLANE), scale));
	}

	// Compute on time for each of the pages
	// Use the fast timer; slow timer is only useful for < 3 shades.
	for (i = 0; i < NUMPLANE; i++) {
		int interval = temp_counts[i + 1] - temp_counts[i];
		counts[i] = 256 - (interval ? interval : 1);
		prescaler[i] = FASTPRESCALER;
	}

	// Compute off time
	int interval = TICKS - (temp_counts[i] >> FASTSCALERSHIFT);
	counts[i] = 256 - (interval ? interval : 1);
	prescaler[i] = SLOWPRESCALER;
}
#endif

/**
 * Distributes the framebuffer content among current cycle pins.
 * @param cycle The cycle whose pattern should to be composed.
 * @param plane The plane ("page" in LoL Shield lingo) to be drawn.
 */
static void compose_cycle(uint8_t const cycle, uint8_t plane) {
	// pointer to corresponding bitmap
	uint8_t *const p = &pixmap[plane][0][0];

#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
	// Set sink pin to Vcc/source, turning off current.
	static uint8_t sink_b = 0, sink_e = 0, sink_g = 0, sink_h = 0;
	PINB = sink_b;
	PINE = sink_e;
	PING = sink_g;
	PINH = sink_h;

	DDRB &= ~0xf0;
	DDRE &= ~0x38;
	DDRG &= ~0x20;
	DDRH &= ~0x78;

	static uint8_t const PROGMEM sink_b_cycle[] =
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0x40, 0x80};
	static uint8_t const PROGMEM sink_e_cycle[] =
		{0x10, 0x20, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static uint8_t const PROGMEM sink_g_cycle[] =
		{0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static uint8_t const PROGMEM sink_h_cycle[] =
		{0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00};

	uint8_t pins_b = sink_b = pgm_read_byte(&sink_b_cycle[cycle]);
	uint8_t pins_e = sink_e = pgm_read_byte(&sink_e_cycle[cycle]);
	uint8_t pins_g = sink_g = pgm_read_byte(&sink_g_cycle[cycle]);
	uint8_t pins_h = sink_h = pgm_read_byte(&sink_h_cycle[cycle]);

	// convert framebuffer to LoL Shield cycles on Arduino Mega 1280/2560
	// NOTE: (0,0) is UPPER RIGHT in the Borgware realm
	if (plane < NUMPLANE) {
		switch(cycle) {
		case 0:
			if (0x02u & p[ 0]) pins_b |= 0x80; // x= 1, y= 0, mapped pin D13
			if (0x02u & p[ 2]) pins_b |= 0x40; // x= 1, y= 1, mapped pin D12
			if (0x02u & p[ 4]) pins_b |= 0x20; // x= 1, y= 2, mapped pin D11
			if (0x02u & p[ 6]) pins_b |= 0x10; // x= 1, y= 3, mapped pin D10
			if (0x02u & p[ 8]) pins_h |= 0x40; // x= 1, y= 4, mapped pin D9
			if (0x02u & p[10]) pins_h |= 0x20; // x= 1, y= 5, mapped pin D8
			if (0x02u & p[12]) pins_h |= 0x10; // x= 1, y= 6, mapped pin D7
			if (0x02u & p[14]) pins_h |= 0x08; // x= 1, y= 7, mapped pin D6
			if (0x02u & p[16]) pins_e |= 0x08; // x= 1, y= 8, mapped pin D5
			break;
		case 1:
			if (0x08u & p[ 0]) pins_b |= 0x80; // x= 3, y= 0, mapped pin D13
			if (0x08u & p[ 2]) pins_b |= 0x40; // x= 3, y= 1, mapped pin D12
			if (0x08u & p[ 4]) pins_b |= 0x20; // x= 3, y= 2, mapped pin D11
			if (0x08u & p[ 6]) pins_b |= 0x10; // x= 3, y= 3, mapped pin D10
			if (0x08u & p[ 8]) pins_h |= 0x40; // x= 3, y= 4, mapped pin D9
			if (0x08u & p[10]) pins_h |= 0x20; // x= 3, y= 5, mapped pin D8
			if (0x08u & p[12]) pins_h |= 0x10; // x= 3, y= 6, mapped pin D7
			pins_e |= (0x08u & p[16]);         // x= 3, y= 8, mapped pin D5
			pins_h |= (0x08u & p[14]);         // x= 3, y= 7, mapped pin D6
			break;
		case 2:
			if (0x20u & p[ 0]) pins_b |= 0x80; // x= 5, y= 0, mapped pin D13
			if (0x20u & p[ 2]) pins_b |= 0x40; // x= 5, y= 1, mapped pin D12
			if (0x20u & p[ 6]) pins_b |= 0x10; // x= 5, y= 3, mapped pin D10
			if (0x20u & p[ 8]) pins_h |= 0x40; // x= 5, y= 4, mapped pin D9
			if (0x20u & p[12]) pins_h |= 0x10; // x= 5, y= 6, mapped pin D7
			if (0x20u & p[14]) pins_h |= 0x08; // x= 5, y= 7, mapped pin D6
			if (0x20u & p[16]) pins_e |= 0x08; // x= 5, y= 8, mapped pin D5
			pins_b |= (0x20u & p[ 4]);         // x= 5, y= 2, mapped pin D11
			pins_h |= (0x20u & p[10]);         // x= 5, y= 5, mapped pin D8
			break;
		case 3:
			if (0x01u & p[16]) pins_e |= 0x10; // x= 0, y= 8, mapped pin D2
			if (0x04u & p[16]) pins_e |= 0x20; // x= 2, y= 8, mapped pin D3
			if (0x10u & p[16]) pins_g |= 0x20; // x= 4, y= 8, mapped pin D4
			if (0x20u & p[ 1]) pins_b |= 0x80; // x=13, y= 0, mapped pin D13
			if (0x20u & p[ 3]) pins_b |= 0x40; // x=13, y= 1, mapped pin D12
			if (0x20u & p[ 7]) pins_b |= 0x10; // x=13, y= 3, mapped pin D10
			if (0x20u & p[ 9]) pins_h |= 0x40; // x=13, y= 4, mapped pin D9
			if (0x20u & p[13]) pins_h |= 0x10; // x=13, y= 6, mapped pin D7
			if (0x20u & p[15]) pins_h |= 0x08; // x=13, y= 7, mapped pin D6
			pins_b |= (0x20u & p[ 5]);         // x=13, y= 2, mapped pin D11
			pins_h |= (0x20u & p[11]);         // x=13, y= 5, mapped pin D8
			break;
		case 4:
			if (0x01u & p[14]) pins_e |= 0x10; // x= 0, y= 7, mapped pin D2
			if (0x04u & p[14]) pins_e |= 0x20; // x= 2, y= 7, mapped pin D3
			if (0x10u & p[ 1]) pins_b |= 0x80; // x=12, y= 0, mapped pin D13
			if (0x10u & p[ 3]) pins_b |= 0x40; // x=12, y= 1, mapped pin D12
			if (0x10u & p[ 5]) pins_b |= 0x20; // x=12, y= 2, mapped pin D11
			if (0x10u & p[ 9]) pins_h |= 0x40; // x=12, y= 4, mapped pin D9
			if (0x10u & p[11]) pins_h |= 0x20; // x=12, y= 5, mapped pin D8
			if (0x10u & p[14]) pins_g |= 0x20; // x= 4, y= 7, mapped pin D4
			if (0x20u & p[17]) pins_e |= 0x08; // x=13, y= 8, mapped pin D5
			pins_b |= (0x10u & p[ 7]);         // x=12, y= 3, mapped pin D10
			pins_h |= (0x10u & p[13]);         // x=12, y= 6, mapped pin D7
			break;
		case 5:
			if (0x01u & p[12]) pins_e |= 0x10; // x= 0, y= 6, mapped pin D2
			if (0x04u & p[12]) pins_e |= 0x20; // x= 2, y= 6, mapped pin D3
			if (0x08u & p[ 1]) pins_b |= 0x80; // x=11, y= 0, mapped pin D13
			if (0x08u & p[ 3]) pins_b |= 0x40; // x=11, y= 1, mapped pin D12
			if (0x08u & p[ 5]) pins_b |= 0x20; // x=11, y= 2, mapped pin D11
			if (0x08u & p[ 7]) pins_b |= 0x10; // x=11, y= 3, mapped pin D10
			if (0x08u & p[ 9]) pins_h |= 0x40; // x=11, y= 4, mapped pin D9
			if (0x08u & p[11]) pins_h |= 0x20; // x=11, y= 5, mapped pin D8
			if (0x10u & p[12]) pins_g |= 0x20; // x= 4, y= 6, mapped pin D4
			if (0x10u & p[15]) pins_h |= 0x08; // x=12, y= 7, mapped pin D6
			if (0x10u & p[17]) pins_e |= 0x08; // x=12, y= 8, mapped pin D5
			break;
		case 6:
			if (0x01u & p[10]) pins_e |= 0x10; // x= 0, y= 5, mapped pin D2
			if (0x04u & p[ 1]) pins_b |= 0x80; // x=10, y= 0, mapped pin D13
			if (0x04u & p[ 3]) pins_b |= 0x40; // x=10, y= 1, mapped pin D12
			if (0x04u & p[ 5]) pins_b |= 0x20; // x=10, y= 2, mapped pin D11
			if (0x04u & p[ 7]) pins_b |= 0x10; // x=10, y= 3, mapped pin D10
			if (0x04u & p[ 9]) pins_h |= 0x40; // x=10, y= 4, mapped pin D9
			if (0x04u & p[10]) pins_e |= 0x20; // x= 2, y= 5, mapped pin D3
			if (0x08u & p[13]) pins_h |= 0x10; // x=11, y= 6, mapped pin D7
			if (0x10u & p[10]) pins_g |= 0x20; // x= 4, y= 5, mapped pin D4
			pins_e |= (0x08u & p[17]);         // x=11, y= 8, mapped pin D5
			pins_h |= (0x08u & p[15]);         // x=11, y= 7, mapped pin D6
			break;
		case 7:
			if (0x01u & p[ 8]) pins_e |= 0x10; // x= 0, y= 4, mapped pin D2
			if (0x02u & p[ 1]) pins_b |= 0x80; // x= 9, y= 0, mapped pin D13
			if (0x02u & p[ 3]) pins_b |= 0x40; // x= 9, y= 1, mapped pin D12
			if (0x02u & p[ 5]) pins_b |= 0x20; // x= 9, y= 2, mapped pin D11
			if (0x02u & p[ 7]) pins_b |= 0x10; // x= 9, y= 3, mapped pin D10
			if (0x04u & p[ 8]) pins_e |= 0x20; // x= 2, y= 4, mapped pin D3
			if (0x04u & p[11]) pins_h |= 0x20; // x=10, y= 5, mapped pin D8
			if (0x04u & p[13]) pins_h |= 0x10; // x=10, y= 6, mapped pin D7
			if (0x04u & p[15]) pins_h |= 0x08; // x=10, y= 7, mapped pin D6
			if (0x04u & p[17]) pins_e |= 0x08; // x=10, y= 8, mapped pin D5
			if (0x10u & p[ 8]) pins_g |= 0x20; // x= 4, y= 4, mapped pin D4
			break;
		case 8:
			if (0x01u & p[ 1]) pins_b |= 0x80; // x= 8, y= 0, mapped pin D13
			if (0x01u & p[ 3]) pins_b |= 0x40; // x= 8, y= 1, mapped pin D12
			if (0x01u & p[ 5]) pins_b |= 0x20; // x= 8, y= 2, mapped pin D11
			if (0x01u & p[ 6]) pins_e |= 0x10; // x= 0, y= 3, mapped pin D2
			if (0x02u & p[ 9]) pins_h |= 0x40; // x= 9, y= 4, mapped pin D9
			if (0x02u & p[11]) pins_h |= 0x20; // x= 9, y= 5, mapped pin D8
			if (0x02u & p[13]) pins_h |= 0x10; // x= 9, y= 6, mapped pin D7
			if (0x02u & p[15]) pins_h |= 0x08; // x= 9, y= 7, mapped pin D6
			if (0x02u & p[17]) pins_e |= 0x08; // x= 9, y= 8, mapped pin D5
			if (0x04u & p[ 6]) pins_e |= 0x20; // x= 2, y= 3, mapped pin D3
			if (0x10u & p[ 6]) pins_g |= 0x20; // x= 4, y= 3, mapped pin D4
			break;
		case 9:
			if (0x01u & p[ 4]) pins_e |= 0x10; // x= 0, y= 2, mapped pin D2
			if (0x01u & p[ 7]) pins_b |= 0x10; // x= 8, y= 3, mapped pin D10
			if (0x01u & p[ 9]) pins_h |= 0x40; // x= 8, y= 4, mapped pin D9
			if (0x01u & p[11]) pins_h |= 0x20; // x= 8, y= 5, mapped pin D8
			if (0x01u & p[13]) pins_h |= 0x10; // x= 8, y= 6, mapped pin D7
			if (0x01u & p[15]) pins_h |= 0x08; // x= 8, y= 7, mapped pin D6
			if (0x01u & p[17]) pins_e |= 0x08; // x= 8, y= 8, mapped pin D5
			if (0x04u & p[ 4]) pins_e |= 0x20; // x= 2, y= 2, mapped pin D3
			if (0x10u & p[ 4]) pins_g |= 0x20; // x= 4, y= 2, mapped pin D4
			if (0x80u & p[ 2]) pins_b |= 0x40; // x= 7, y= 1, mapped pin D12
			pins_b |= (0x80u & p[ 0]);         // x= 7, y= 0, mapped pin D13
			break;
		case 10:
			if (0x01u & p[ 2]) pins_e |= 0x10; // x= 0, y= 1, mapped pin D2
			if (0x04u & p[ 2]) pins_e |= 0x20; // x= 2, y= 1, mapped pin D3
			if (0x10u & p[ 2]) pins_g |= 0x20; // x= 4, y= 1, mapped pin D4
			if (0x40u & p[ 0]) pins_b |= 0x80; // x= 6, y= 0, mapped pin D13
			if (0x80u & p[ 4]) pins_b |= 0x20; // x= 7, y= 2, mapped pin D11
			if (0x80u & p[ 6]) pins_b |= 0x10; // x= 7, y= 3, mapped pin D10
			if (0x80u & p[ 8]) pins_h |= 0x40; // x= 7, y= 4, mapped pin D9
			if (0x80u & p[10]) pins_h |= 0x20; // x= 7, y= 5, mapped pin D8
			if (0x80u & p[12]) pins_h |= 0x10; // x= 7, y= 6, mapped pin D7
			if (0x80u & p[14]) pins_h |= 0x08; // x= 7, y= 7, mapped pin D6
			if (0x80u & p[16]) pins_e |= 0x08; // x= 7, y= 8, mapped pin D5
			break;
		case 11:
			if (0x01u & p[ 0]) pins_e |= 0x10; // x= 0, y= 0, mapped pin D2
			if (0x04u & p[ 0]) pins_e |= 0x20; // x= 2, y= 0, mapped pin D3
			if (0x10u & p[ 0]) pins_g |= 0x20; // x= 4, y= 0, mapped pin D4
			if (0x40u & p[ 4]) pins_b |= 0x20; // x= 6, y= 2, mapped pin D11
			if (0x40u & p[ 6]) pins_b |= 0x10; // x= 6, y= 3, mapped pin D10
			if (0x40u & p[10]) pins_h |= 0x20; // x= 6, y= 5, mapped pin D8
			if (0x40u & p[12]) pins_h |= 0x10; // x= 6, y= 6, mapped pin D7
			if (0x40u & p[14]) pins_h |= 0x08; // x= 6, y= 7, mapped pin D6
			if (0x40u & p[16]) pins_e |= 0x08; // x= 6, y= 8, mapped pin D5
			pins_b |= (0x40u & p[ 2]);         // x= 6, y= 1, mapped pin D12
			pins_h |= (0x40u & p[ 8]);         // x= 6, y= 4, mapped pin D9
			break;
		}
	}

	// Enable pullups (by toggling) on new output pins.
	PINB = PORTB ^ pins_b;
	PINE = PORTE ^ pins_e;
	PING = PORTG ^ pins_g;
	PINH = PORTH ^ pins_h;

	// Set pins to output mode; pullups become Vcc/source.
	DDRB |= pins_b;
	DDRE |= pins_e;
	DDRG |= pins_g;
	DDRH |= pins_h;

	// Set sink pin to GND/sink, turning on current.
	PINB = sink_b;
	PINE = sink_e;
	PING = sink_g;
	PINH = sink_h;
#elif defined (__AVR_ATmega32U4__)
	// Set sink pin to Vcc/source, turning off current.
	static uint8_t sink_b = 0, sink_c = 0, sink_d = 0, sink_e = 0;
	PINB = sink_b;
	PINC = sink_c;
	PIND = sink_d;
	PINE = sink_e;

	DDRB &= ~0xF0;
	DDRC &= ~0xC0;
	DDRD &= ~0xD3;
	DDRE &= ~0x40;

	static uint8_t const PROGMEM sink_b_cycle[] =
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00};
	static uint8_t const PROGMEM sink_c_cycle[] =
		{0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
	static uint8_t const PROGMEM sink_d_cycle[] =
		{0x02, 0x01, 0x10, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00};
	static uint8_t const PROGMEM sink_e_cycle[] =
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	uint8_t pins_b = sink_b = pgm_read_byte(&sink_b_cycle[cycle]);
	uint8_t pins_c = sink_c = pgm_read_byte(&sink_c_cycle[cycle]);
	uint8_t pins_d = sink_d = pgm_read_byte(&sink_d_cycle[cycle]);
	uint8_t pins_e = sink_e = pgm_read_byte(&sink_e_cycle[cycle]);

	// convert Borgware-2D framebuffer to LoL Shield cycles on Arduino Leonardo
	// NOTE: (0,0) is UPPER RIGHT in the Borgware realm
	if (plane < NUMPLANE) {
		switch(cycle) {
		case 0:
			if (0x02u & p[ 0]) pins_c |= 0x80; // x= 1, y= 0, mapped pin D13
			if (0x02u & p[ 2]) pins_d |= 0x40; // x= 1, y= 1, mapped pin D12
			if (0x02u & p[ 4]) pins_b |= 0x80; // x= 1, y= 2, mapped pin D11
			if (0x02u & p[ 6]) pins_b |= 0x40; // x= 1, y= 3, mapped pin D10
			if (0x02u & p[ 8]) pins_b |= 0x20; // x= 1, y= 4, mapped pin D9
			if (0x02u & p[10]) pins_b |= 0x10; // x= 1, y= 5, mapped pin D8
			if (0x02u & p[12]) pins_e |= 0x40; // x= 1, y= 6, mapped pin D7
			if (0x02u & p[14]) pins_d |= 0x80; // x= 1, y= 7, mapped pin D6
			if (0x02u & p[16]) pins_c |= 0x40; // x= 1, y= 8, mapped pin D5
			break;
		case 1:
			if (0x08u & p[ 0]) pins_c |= 0x80; // x= 3, y= 0, mapped pin D13
			if (0x08u & p[ 2]) pins_d |= 0x40; // x= 3, y= 1, mapped pin D12
			if (0x08u & p[ 4]) pins_b |= 0x80; // x= 3, y= 2, mapped pin D11
			if (0x08u & p[ 6]) pins_b |= 0x40; // x= 3, y= 3, mapped pin D10
			if (0x08u & p[ 8]) pins_b |= 0x20; // x= 3, y= 4, mapped pin D9
			if (0x08u & p[10]) pins_b |= 0x10; // x= 3, y= 5, mapped pin D8
			if (0x08u & p[12]) pins_e |= 0x40; // x= 3, y= 6, mapped pin D7
			if (0x08u & p[14]) pins_d |= 0x80; // x= 3, y= 7, mapped pin D6
			if (0x08u & p[16]) pins_c |= 0x40; // x= 3, y= 8, mapped pin D5
			break;
		case 2:
			if (0x20u & p[ 0]) pins_c |= 0x80; // x= 5, y= 0, mapped pin D13
			if (0x20u & p[ 2]) pins_d |= 0x40; // x= 5, y= 1, mapped pin D12
			if (0x20u & p[ 4]) pins_b |= 0x80; // x= 5, y= 2, mapped pin D11
			if (0x20u & p[ 6]) pins_b |= 0x40; // x= 5, y= 3, mapped pin D10
			if (0x20u & p[10]) pins_b |= 0x10; // x= 5, y= 5, mapped pin D8
			if (0x20u & p[12]) pins_e |= 0x40; // x= 5, y= 6, mapped pin D7
			if (0x20u & p[14]) pins_d |= 0x80; // x= 5, y= 7, mapped pin D6
			if (0x20u & p[16]) pins_c |= 0x40; // x= 5, y= 8, mapped pin D5
			pins_b |= (0x20u & p[ 8]);         // x= 5, y= 4, mapped pin D9
			break;
		case 3:
			if (0x01u & p[16]) pins_d |= 0x02; // x= 0, y= 8, mapped pin D2
			if (0x04u & p[16]) pins_d |= 0x01; // x= 2, y= 8, mapped pin D3
			if (0x20u & p[ 1]) pins_c |= 0x80; // x=13, y= 0, mapped pin D13
			if (0x20u & p[ 3]) pins_d |= 0x40; // x=13, y= 1, mapped pin D12
			if (0x20u & p[ 5]) pins_b |= 0x80; // x=13, y= 2, mapped pin D11
			if (0x20u & p[ 7]) pins_b |= 0x40; // x=13, y= 3, mapped pin D10
			if (0x20u & p[11]) pins_b |= 0x10; // x=13, y= 5, mapped pin D8
			if (0x20u & p[13]) pins_e |= 0x40; // x=13, y= 6, mapped pin D7
			if (0x20u & p[15]) pins_d |= 0x80; // x=13, y= 7, mapped pin D6
			pins_b |= (0x20u & p[ 9]);         // x=13, y= 4, mapped pin D9
			pins_d |= (0x10u & p[16]);         // x= 4, y= 8, mapped pin D4
			break;
		case 4:
			if (0x01u & p[14]) pins_d |= 0x02; // x= 0, y= 7, mapped pin D2
			if (0x04u & p[14]) pins_d |= 0x01; // x= 2, y= 7, mapped pin D3
			if (0x10u & p[ 1]) pins_c |= 0x80; // x=12, y= 0, mapped pin D13
			if (0x10u & p[ 3]) pins_d |= 0x40; // x=12, y= 1, mapped pin D12
			if (0x10u & p[ 5]) pins_b |= 0x80; // x=12, y= 2, mapped pin D11
			if (0x10u & p[ 7]) pins_b |= 0x40; // x=12, y= 3, mapped pin D10
			if (0x10u & p[ 9]) pins_b |= 0x20; // x=12, y= 4, mapped pin D9
			if (0x10u & p[13]) pins_e |= 0x40; // x=12, y= 6, mapped pin D7
			if (0x20u & p[17]) pins_c |= 0x40; // x=13, y= 8, mapped pin D5
			pins_b |= (0x10u & p[11]);         // x=12, y= 5, mapped pin D8
			pins_d |= (0x10u & p[14]);         // x= 4, y= 7, mapped pin D4
			break;
		case 5:
			if (0x01u & p[12]) pins_d |= 0x02; // x= 0, y= 6, mapped pin D2
			if (0x04u & p[12]) pins_d |= 0x01; // x= 2, y= 6, mapped pin D3
			if (0x08u & p[ 1]) pins_c |= 0x80; // x=11, y= 0, mapped pin D13
			if (0x08u & p[ 3]) pins_d |= 0x40; // x=11, y= 1, mapped pin D12
			if (0x08u & p[ 5]) pins_b |= 0x80; // x=11, y= 2, mapped pin D11
			if (0x08u & p[ 7]) pins_b |= 0x40; // x=11, y= 3, mapped pin D10
			if (0x08u & p[ 9]) pins_b |= 0x20; // x=11, y= 4, mapped pin D9
			if (0x08u & p[11]) pins_b |= 0x10; // x=11, y= 5, mapped pin D8
			if (0x10u & p[15]) pins_d |= 0x80; // x=12, y= 7, mapped pin D6
			if (0x10u & p[17]) pins_c |= 0x40; // x=12, y= 8, mapped pin D5
			pins_d |= (0x10u & p[12]);         // x= 4, y= 6, mapped pin D4
			break;
		case 6:
			if (0x01u & p[10]) pins_d |= 0x02; // x= 0, y= 5, mapped pin D2
			if (0x04u & p[ 1]) pins_c |= 0x80; // x=10, y= 0, mapped pin D13
			if (0x04u & p[ 3]) pins_d |= 0x40; // x=10, y= 1, mapped pin D12
			if (0x04u & p[ 5]) pins_b |= 0x80; // x=10, y= 2, mapped pin D11
			if (0x04u & p[ 7]) pins_b |= 0x40; // x=10, y= 3, mapped pin D10
			if (0x04u & p[ 9]) pins_b |= 0x20; // x=10, y= 4, mapped pin D9
			if (0x04u & p[10]) pins_d |= 0x01; // x= 2, y= 5, mapped pin D3
			if (0x08u & p[13]) pins_e |= 0x40; // x=11, y= 6, mapped pin D7
			if (0x08u & p[15]) pins_d |= 0x80; // x=11, y= 7, mapped pin D6
			if (0x08u & p[17]) pins_c |= 0x40; // x=11, y= 8, mapped pin D5
			pins_d |= (0x10u & p[10]);         // x= 4, y= 5, mapped pin D4
			break;
		case 7:
			if (0x01u & p[ 8]) pins_d |= 0x02; // x= 0, y= 4, mapped pin D2
			if (0x02u & p[ 1]) pins_c |= 0x80; // x= 9, y= 0, mapped pin D13
			if (0x02u & p[ 3]) pins_d |= 0x40; // x= 9, y= 1, mapped pin D12
			if (0x02u & p[ 5]) pins_b |= 0x80; // x= 9, y= 2, mapped pin D11
			if (0x02u & p[ 7]) pins_b |= 0x40; // x= 9, y= 3, mapped pin D10
			if (0x04u & p[ 8]) pins_d |= 0x01; // x= 2, y= 4, mapped pin D3
			if (0x04u & p[11]) pins_b |= 0x10; // x=10, y= 5, mapped pin D8
			if (0x04u & p[13]) pins_e |= 0x40; // x=10, y= 6, mapped pin D7
			if (0x04u & p[15]) pins_d |= 0x80; // x=10, y= 7, mapped pin D6
			if (0x04u & p[17]) pins_c |= 0x40; // x=10, y= 8, mapped pin D5
			pins_d |= (0x10u & p[ 8]);         // x= 4, y= 4, mapped pin D4
			break;
		case 8:
			if (0x01u & p[ 1]) pins_c |= 0x80; // x= 8, y= 0, mapped pin D13
			if (0x01u & p[ 3]) pins_d |= 0x40; // x= 8, y= 1, mapped pin D12
			if (0x01u & p[ 5]) pins_b |= 0x80; // x= 8, y= 2, mapped pin D11
			if (0x01u & p[ 6]) pins_d |= 0x02; // x= 0, y= 3, mapped pin D2
			if (0x02u & p[ 9]) pins_b |= 0x20; // x= 9, y= 4, mapped pin D9
			if (0x02u & p[11]) pins_b |= 0x10; // x= 9, y= 5, mapped pin D8
			if (0x02u & p[13]) pins_e |= 0x40; // x= 9, y= 6, mapped pin D7
			if (0x02u & p[15]) pins_d |= 0x80; // x= 9, y= 7, mapped pin D6
			if (0x02u & p[17]) pins_c |= 0x40; // x= 9, y= 8, mapped pin D5
			if (0x04u & p[ 6]) pins_d |= 0x01; // x= 2, y= 3, mapped pin D3
			pins_d |= (0x10u & p[ 6]);         // x= 4, y= 3, mapped pin D4
			break;
		case 9:
			if (0x01u & p[ 4]) pins_d |= 0x02; // x= 0, y= 2, mapped pin D2
			if (0x01u & p[ 7]) pins_b |= 0x40; // x= 8, y= 3, mapped pin D10
			if (0x01u & p[ 9]) pins_b |= 0x20; // x= 8, y= 4, mapped pin D9
			if (0x01u & p[11]) pins_b |= 0x10; // x= 8, y= 5, mapped pin D8
			if (0x01u & p[13]) pins_e |= 0x40; // x= 8, y= 6, mapped pin D7
			if (0x01u & p[15]) pins_d |= 0x80; // x= 8, y= 7, mapped pin D6
			if (0x01u & p[17]) pins_c |= 0x40; // x= 8, y= 8, mapped pin D5
			if (0x04u & p[ 4]) pins_d |= 0x01; // x= 2, y= 2, mapped pin D3
			if (0x80u & p[ 2]) pins_d |= 0x40; // x= 7, y= 1, mapped pin D12
			pins_c |= (0x80u & p[ 0]);         // x= 7, y= 0, mapped pin D13
			pins_d |= (0x10u & p[ 4]);         // x= 4, y= 2, mapped pin D4
			break;
		case 10:
			if (0x01u & p[ 2]) pins_d |= 0x02; // x= 0, y= 1, mapped pin D2
			if (0x04u & p[ 2]) pins_d |= 0x01; // x= 2, y= 1, mapped pin D3
			if (0x40u & p[ 0]) pins_c |= 0x80; // x= 6, y= 0, mapped pin D13
			if (0x80u & p[ 6]) pins_b |= 0x40; // x= 7, y= 3, mapped pin D10
			if (0x80u & p[ 8]) pins_b |= 0x20; // x= 7, y= 4, mapped pin D9
			if (0x80u & p[10]) pins_b |= 0x10; // x= 7, y= 5, mapped pin D8
			if (0x80u & p[12]) pins_e |= 0x40; // x= 7, y= 6, mapped pin D7
			if (0x80u & p[16]) pins_c |= 0x40; // x= 7, y= 8, mapped pin D5
			pins_b |= (0x80u & p[ 4]);         // x= 7, y= 2, mapped pin D11
			pins_d |= (0x10u & p[ 2]);         // x= 4, y= 1, mapped pin D4
			pins_d |= (0x80u & p[14]);         // x= 7, y= 7, mapped pin D6
			break;
		case 11:
			if (0x01u & p[ 0]) pins_d |= 0x02; // x= 0, y= 0, mapped pin D2
			if (0x04u & p[ 0]) pins_d |= 0x01; // x= 2, y= 0, mapped pin D3
			if (0x40u & p[ 4]) pins_b |= 0x80; // x= 6, y= 2, mapped pin D11
			if (0x40u & p[ 8]) pins_b |= 0x20; // x= 6, y= 4, mapped pin D9
			if (0x40u & p[10]) pins_b |= 0x10; // x= 6, y= 5, mapped pin D8
			if (0x40u & p[14]) pins_d |= 0x80; // x= 6, y= 7, mapped pin D6
			pins_b |= (0x40u & p[ 6]);         // x= 6, y= 3, mapped pin D10
			pins_c |= (0x40u & p[16]);         // x= 6, y= 8, mapped pin D5
			pins_d |= (0x10u & p[ 0]);         // x= 4, y= 0, mapped pin D4
			pins_d |= (0x40u & p[ 2]);         // x= 6, y= 1, mapped pin D12
			pins_e |= (0x40u & p[12]);         // x= 6, y= 6, mapped pin D7
			break;
		}
	}

	// Enable pullups (by toggling) on new output pins.
	PINB = PORTB ^ pins_b;
	PINC = PORTC ^ pins_c;
	PIND = PORTD ^ pins_d;
	PINE = PORTE ^ pins_e;

	// Set pins to output mode; pullups become Vcc/source.
	DDRB |= pins_b;
	DDRC |= pins_c;
	DDRD |= pins_d;
	DDRE |= pins_e;

	// Set sink pin to GND/sink, turning on current.
	PINB = sink_b;
	PINC = sink_c;
	PIND = sink_d;
	PINE = sink_e;
#else
	// Set sink pin to Vcc/source, turning off current.
	static uint8_t sink_b = 0, sink_d = 0;
	PIND = sink_d;
	PINB = sink_b;

	// Set pins to input mode; Vcc/source become pullups.
	DDRD = 0;
	DDRB = 0;

	static uint8_t const PROGMEM sink_d_cycle[] =
		{0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static uint8_t const PROGMEM sink_b_cycle[] =
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20};

	uint8_t pins_d = sink_d = pgm_read_byte(&sink_d_cycle[cycle]);
	uint8_t pins_b = sink_b = pgm_read_byte(&sink_b_cycle[cycle]);

	// convert Borgware-2D framebuffer to LoL Shield cycles on Diavolino
	// NOTE: (0,0) is UPPER RIGHT in the Borgware realm
	if (plane < NUMPLANE) {
		switch(cycle) {
		case 0:
			if (0x02u & p[ 0]) pins_b |= 0x20; // x= 1, y= 0, mapped pin D13
			if (0x02u & p[ 2]) pins_b |= 0x10; // x= 1, y= 1, mapped pin D12
			if (0x02u & p[ 4]) pins_b |= 0x08; // x= 1, y= 2, mapped pin D11
			if (0x02u & p[ 6]) pins_b |= 0x04; // x= 1, y= 3, mapped pin D10
			if (0x02u & p[10]) pins_b |= 0x01; // x= 1, y= 5, mapped pin D8
			if (0x02u & p[12]) pins_d |= 0x80; // x= 1, y= 6, mapped pin D7
			if (0x02u & p[14]) pins_d |= 0x40; // x= 1, y= 7, mapped pin D6
			if (0x02u & p[16]) pins_d |= 0x20; // x= 1, y= 8, mapped pin D5
			pins_b |= (0x02u & p[ 8]);         // x= 1, y= 4, mapped pin D9
			break;
		case 1:
			if (0x08u & p[ 0]) pins_b |= 0x20; // x= 3, y= 0, mapped pin D13
			if (0x08u & p[ 2]) pins_b |= 0x10; // x= 3, y= 1, mapped pin D12
			if (0x08u & p[ 6]) pins_b |= 0x04; // x= 3, y= 3, mapped pin D10
			if (0x08u & p[ 8]) pins_b |= 0x02; // x= 3, y= 4, mapped pin D9
			if (0x08u & p[10]) pins_b |= 0x01; // x= 3, y= 5, mapped pin D8
			if (0x08u & p[12]) pins_d |= 0x80; // x= 3, y= 6, mapped pin D7
			if (0x08u & p[14]) pins_d |= 0x40; // x= 3, y= 7, mapped pin D6
			if (0x08u & p[16]) pins_d |= 0x20; // x= 3, y= 8, mapped pin D5
			pins_b |= (0x08u & p[ 4]);         // x= 3, y= 2, mapped pin D11
			break;
		case 2:
			if (0x20u & p[ 2]) pins_b |= 0x10; // x= 5, y= 1, mapped pin D12
			if (0x20u & p[ 4]) pins_b |= 0x08; // x= 5, y= 2, mapped pin D11
			if (0x20u & p[ 6]) pins_b |= 0x04; // x= 5, y= 3, mapped pin D10
			if (0x20u & p[ 8]) pins_b |= 0x02; // x= 5, y= 4, mapped pin D9
			if (0x20u & p[10]) pins_b |= 0x01; // x= 5, y= 5, mapped pin D8
			if (0x20u & p[12]) pins_d |= 0x80; // x= 5, y= 6, mapped pin D7
			if (0x20u & p[14]) pins_d |= 0x40; // x= 5, y= 7, mapped pin D6
			pins_b |= (0x20u & p[ 0]);         // x= 5, y= 0, mapped pin D13
			pins_d |= (0x20u & p[16]);         // x= 5, y= 8, mapped pin D5
			break;
		case 3:
			if (0x01u & p[16]) pins_d |= 0x04; // x= 0, y= 8, mapped pin D2
			if (0x04u & p[16]) pins_d |= 0x08; // x= 2, y= 8, mapped pin D3
			if (0x20u & p[ 3]) pins_b |= 0x10; // x=13, y= 1, mapped pin D12
			if (0x20u & p[ 5]) pins_b |= 0x08; // x=13, y= 2, mapped pin D11
			if (0x20u & p[ 7]) pins_b |= 0x04; // x=13, y= 3, mapped pin D10
			if (0x20u & p[ 9]) pins_b |= 0x02; // x=13, y= 4, mapped pin D9
			if (0x20u & p[11]) pins_b |= 0x01; // x=13, y= 5, mapped pin D8
			if (0x20u & p[13]) pins_d |= 0x80; // x=13, y= 6, mapped pin D7
			if (0x20u & p[15]) pins_d |= 0x40; // x=13, y= 7, mapped pin D6
			pins_b |= (0x20u & p[ 1]);         // x=13, y= 0, mapped pin D13
			pins_d |= (0x10u & p[16]);         // x= 4, y= 8, mapped pin D4
			break;
		case 4:
			if (0x01u & p[14]) pins_d |= 0x04; // x= 0, y= 7, mapped pin D2
			if (0x04u & p[14]) pins_d |= 0x08; // x= 2, y= 7, mapped pin D3
			if (0x10u & p[ 1]) pins_b |= 0x20; // x=12, y= 0, mapped pin D13
			if (0x10u & p[ 5]) pins_b |= 0x08; // x=12, y= 2, mapped pin D11
			if (0x10u & p[ 7]) pins_b |= 0x04; // x=12, y= 3, mapped pin D10
			if (0x10u & p[ 9]) pins_b |= 0x02; // x=12, y= 4, mapped pin D9
			if (0x10u & p[11]) pins_b |= 0x01; // x=12, y= 5, mapped pin D8
			if (0x10u & p[13]) pins_d |= 0x80; // x=12, y= 6, mapped pin D7
			pins_b |= (0x10u & p[ 3]);         // x=12, y= 1, mapped pin D12
			pins_d |= (0x10u & p[14]);         // x= 4, y= 7, mapped pin D4
			pins_d |= (0x20u & p[17]);         // x=13, y= 8, mapped pin D5
			break;
		case 5:
			if (0x01u & p[12]) pins_d |= 0x04; // x= 0, y= 6, mapped pin D2
			if (0x04u & p[12]) pins_d |= 0x08; // x= 2, y= 6, mapped pin D3
			if (0x08u & p[ 1]) pins_b |= 0x20; // x=11, y= 0, mapped pin D13
			if (0x08u & p[ 3]) pins_b |= 0x10; // x=11, y= 1, mapped pin D12
			if (0x08u & p[ 7]) pins_b |= 0x04; // x=11, y= 3, mapped pin D10
			if (0x08u & p[ 9]) pins_b |= 0x02; // x=11, y= 4, mapped pin D9
			if (0x08u & p[11]) pins_b |= 0x01; // x=11, y= 5, mapped pin D8
			if (0x10u & p[15]) pins_d |= 0x40; // x=12, y= 7, mapped pin D6
			if (0x10u & p[17]) pins_d |= 0x20; // x=12, y= 8, mapped pin D5
			pins_b |= (0x08u & p[ 5]);         // x=11, y= 2, mapped pin D11
			pins_d |= (0x10u & p[12]);         // x= 4, y= 6, mapped pin D4
			break;
		case 6:
			if (0x01u & p[10]) pins_d |= 0x04; // x= 0, y= 5, mapped pin D2
			if (0x04u & p[ 1]) pins_b |= 0x20; // x=10, y= 0, mapped pin D13
			if (0x04u & p[ 3]) pins_b |= 0x10; // x=10, y= 1, mapped pin D12
			if (0x04u & p[ 5]) pins_b |= 0x08; // x=10, y= 2, mapped pin D11
			if (0x04u & p[ 9]) pins_b |= 0x02; // x=10, y= 4, mapped pin D9
			if (0x04u & p[10]) pins_d |= 0x08; // x= 2, y= 5, mapped pin D3
			if (0x08u & p[13]) pins_d |= 0x80; // x=11, y= 6, mapped pin D7
			if (0x08u & p[15]) pins_d |= 0x40; // x=11, y= 7, mapped pin D6
			if (0x08u & p[17]) pins_d |= 0x20; // x=11, y= 8, mapped pin D5
			pins_b |= (0x04u & p[ 7]);         // x=10, y= 3, mapped pin D10
			pins_d |= (0x10u & p[10]);         // x= 4, y= 5, mapped pin D4
			break;
		case 7:
			if (0x01u & p[ 8]) pins_d |= 0x04; // x= 0, y= 4, mapped pin D2
			if (0x02u & p[ 1]) pins_b |= 0x20; // x= 9, y= 0, mapped pin D13
			if (0x02u & p[ 3]) pins_b |= 0x10; // x= 9, y= 1, mapped pin D12
			if (0x02u & p[ 5]) pins_b |= 0x08; // x= 9, y= 2, mapped pin D11
			if (0x02u & p[ 7]) pins_b |= 0x04; // x= 9, y= 3, mapped pin D10
			if (0x04u & p[ 8]) pins_d |= 0x08; // x= 2, y= 4, mapped pin D3
			if (0x04u & p[11]) pins_b |= 0x01; // x=10, y= 5, mapped pin D8
			if (0x04u & p[13]) pins_d |= 0x80; // x=10, y= 6, mapped pin D7
			if (0x04u & p[15]) pins_d |= 0x40; // x=10, y= 7, mapped pin D6
			if (0x04u & p[17]) pins_d |= 0x20; // x=10, y= 8, mapped pin D5
			pins_d |= (0x10u & p[ 8]);         // x= 4, y= 4, mapped pin D4
			break;
		case 8:
			if (0x01u & p[ 1]) pins_b |= 0x20; // x= 8, y= 0, mapped pin D13
			if (0x01u & p[ 3]) pins_b |= 0x10; // x= 8, y= 1, mapped pin D12
			if (0x01u & p[ 5]) pins_b |= 0x08; // x= 8, y= 2, mapped pin D11
			if (0x01u & p[ 6]) pins_d |= 0x04; // x= 0, y= 3, mapped pin D2
			if (0x02u & p[11]) pins_b |= 0x01; // x= 9, y= 5, mapped pin D8
			if (0x02u & p[13]) pins_d |= 0x80; // x= 9, y= 6, mapped pin D7
			if (0x02u & p[15]) pins_d |= 0x40; // x= 9, y= 7, mapped pin D6
			if (0x02u & p[17]) pins_d |= 0x20; // x= 9, y= 8, mapped pin D5
			if (0x04u & p[ 6]) pins_d |= 0x08; // x= 2, y= 3, mapped pin D3
			pins_b |= (0x02u & p[ 9]);         // x= 9, y= 4, mapped pin D9
			pins_d |= (0x10u & p[ 6]);         // x= 4, y= 3, mapped pin D4
			break;
		case 9:
			if (0x01u & p[ 4]) pins_d |= 0x04; // x= 0, y= 2, mapped pin D2
			if (0x01u & p[ 7]) pins_b |= 0x04; // x= 8, y= 3, mapped pin D10
			if (0x01u & p[ 9]) pins_b |= 0x02; // x= 8, y= 4, mapped pin D9
			if (0x01u & p[13]) pins_d |= 0x80; // x= 8, y= 6, mapped pin D7
			if (0x01u & p[15]) pins_d |= 0x40; // x= 8, y= 7, mapped pin D6
			if (0x01u & p[17]) pins_d |= 0x20; // x= 8, y= 8, mapped pin D5
			if (0x04u & p[ 4]) pins_d |= 0x08; // x= 2, y= 2, mapped pin D3
			if (0x80u & p[ 0]) pins_b |= 0x20; // x= 7, y= 0, mapped pin D13
			if (0x80u & p[ 2]) pins_b |= 0x10; // x= 7, y= 1, mapped pin D12
			pins_b |= (0x01u & p[11]);         // x= 8, y= 5, mapped pin D8
			pins_d |= (0x10u & p[ 4]);         // x= 4, y= 2, mapped pin D4
			break;
		case 10:
			if (0x01u & p[ 2]) pins_d |= 0x04; // x= 0, y= 1, mapped pin D2
			if (0x04u & p[ 2]) pins_d |= 0x08; // x= 2, y= 1, mapped pin D3
			if (0x40u & p[ 0]) pins_b |= 0x20; // x= 6, y= 0, mapped pin D13
			if (0x80u & p[ 4]) pins_b |= 0x08; // x= 7, y= 2, mapped pin D11
			if (0x80u & p[ 6]) pins_b |= 0x04; // x= 7, y= 3, mapped pin D10
			if (0x80u & p[ 8]) pins_b |= 0x02; // x= 7, y= 4, mapped pin D9
			if (0x80u & p[10]) pins_b |= 0x01; // x= 7, y= 5, mapped pin D8
			if (0x80u & p[14]) pins_d |= 0x40; // x= 7, y= 7, mapped pin D6
			if (0x80u & p[16]) pins_d |= 0x20; // x= 7, y= 8, mapped pin D5
			pins_d |= (0x10u & p[ 2]);         // x= 4, y= 1, mapped pin D4
			pins_d |= (0x80u & p[12]);         // x= 7, y= 6, mapped pin D7
			break;
		case 11:
			if (0x01u & p[ 0]) pins_d |= 0x04; // x= 0, y= 0, mapped pin D2
			if (0x04u & p[ 0]) pins_d |= 0x08; // x= 2, y= 0, mapped pin D3
			if (0x40u & p[ 2]) pins_b |= 0x10; // x= 6, y= 1, mapped pin D12
			if (0x40u & p[ 4]) pins_b |= 0x08; // x= 6, y= 2, mapped pin D11
			if (0x40u & p[ 6]) pins_b |= 0x04; // x= 6, y= 3, mapped pin D10
			if (0x40u & p[ 8]) pins_b |= 0x02; // x= 6, y= 4, mapped pin D9
			if (0x40u & p[10]) pins_b |= 0x01; // x= 6, y= 5, mapped pin D8
			if (0x40u & p[12]) pins_d |= 0x80; // x= 6, y= 6, mapped pin D7
			if (0x40u & p[16]) pins_d |= 0x20; // x= 6, y= 8, mapped pin D5
			pins_d |= (0x10u & p[ 0]);         // x= 4, y= 0, mapped pin D4
			pins_d |= (0x40u & p[14]);         // x= 6, y= 7, mapped pin D6
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
#endif
}

#if !defined (__AVR_ATmega32U4__)
ISR(TIMER2_OVF_vect) {
#else
ISR(TIMER1_COMPA_vect) {
#endif
	// For each cycle, we have potential planes to display. Once every plane has
	// been displayed, then we move on to the next cycle.
	// NOTE: a "cycle" is a subset of LEDs that can be driven at once.

	// 12 Cycles of Matrix
	static uint8_t cycle = 0;

	// planes to display
	// NOTE: a "plane" in the Borgware is the same as a "page" in Jimmie's lib
	static uint8_t plane = 0;

#if defined (__AVR_ATmega48__)   || \
    defined (__AVR_ATmega48P__)  || \
    defined (__AVR_ATmega88__)   || \
    defined (__AVR_ATmega88P__)  || \
    defined (__AVR_ATmega168__)  || \
    defined (__AVR_ATmega168P__) || \
    defined (__AVR_ATmega328__)  || \
    defined (__AVR_ATmega328P__) || \
    defined (__AVR_ATmega1280__) || \
    defined (__AVR_ATmega2560__)
	TCCR2B = prescaler[plane];
#elif defined (__AVR_ATmega8__) || \
      defined (__AVR_ATmega128__)
	TCCR2 = prescaler[page];
#elif defined (__AVR_ATmega32U4__)
	TCCR1B = prescaler[plane];
#endif
#if !defined (__AVR_ATmega32U4__)
	TCNT2 = counts[plane];
#else
	TCNT1 = counts[plane];
#endif

	// distribute framebuffer contents among current cycle pins
	compose_cycle(cycle, plane++);

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

#if defined (__AVR_ATmega48__)   || \
    defined (__AVR_ATmega48P__)  || \
    defined (__AVR_ATmega88__)   || \
    defined (__AVR_ATmega88P__)  || \
    defined (__AVR_ATmega168__)  || \
    defined (__AVR_ATmega168P__) || \
    defined (__AVR_ATmega328__)  || \
    defined (__AVR_ATmega328P__) || \
    defined (__AVR_ATmega1280__) || \
    defined (__AVR_ATmega2560__)
	TIMSK2 &= ~(_BV(TOIE2) | _BV(OCIE2A));
	TCCR2A &= ~(_BV(WGM21) | _BV(WGM20));
	TCCR2B &= ~_BV(WGM22);
	ASSR &= ~_BV(AS2);
#elif defined (__AVR_ATmega8__)
	TIMSK &= ~(_BV(TOIE2) | _BV(OCIE2));
	TCCR2 &= ~(_BV(WGM21) | _BV(WGM20));
	ASSR &= ~_BV(AS2);
#elif defined (__AVR_ATmega128__)
	TIMSK &= ~(_BV(TOIE2) | _BV(OCIE2));
	TCCR2 &= ~(_BV(WGM21) | _BV(WGM20));
#elif defined (__AVR_ATmega32U4__)
	// The only 8bit timer on the Leonardo is used by default, so we use the 16bit Timer1
	// in CTC mode with a compare value of 256 to achieve the same behaviour.
	TIMSK1 &= ~(_BV(TOIE1) | _BV(OCIE1A));
	TCCR1A &= ~(_BV(WGM10) | _BV(WGM11));
	OCR1A = 256;
#endif

#if NUMPLANE >= 8
	setBrightness();
#endif

	// Then start the display
#if defined (__AVR_ATmega48__)   || \
    defined (__AVR_ATmega48P__)  || \
    defined (__AVR_ATmega88__)   || \
    defined (__AVR_ATmega88P__)  || \
    defined (__AVR_ATmega168__)  || \
    defined (__AVR_ATmega168P__) || \
    defined (__AVR_ATmega328__)  || \
    defined (__AVR_ATmega328P__) || \
    defined (__AVR_ATmega1280__) || \
    defined (__AVR_ATmega2560__)
	TIMSK2 |= _BV(TOIE2);
	TCCR2B = FASTPRESCALER;
#elif defined (__AVR_ATmega8__) || \
      defined (__AVR_ATmega128__)
	TIMSK |= _BV(TOIE2);
	TCCR2 = FASTPRESCALER;
#elif defined (__AVR_ATmega32U4__)
	// Enable output compare match interrupt
	TIMSK1 |= _BV(OCIE1A);
	TCCR1B = FASTPRESCALER;
#endif
	// interrupt ASAP
#if !defined (__AVR_ATmega32U4__)
		TCNT2 = 255;
#else
		TCNT1 = 255;
#endif

	// activate watchdog timer
	wdt_reset();
	wdt_enable(WDTO_15MS); // 15ms watchdog
}
