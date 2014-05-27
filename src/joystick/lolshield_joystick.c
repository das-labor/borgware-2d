/**
 * @file lolshield_joystick.c
 * @brief Driver for Jimmie Rodgers' LoL Shield
 * @author Christian Kroll
 * @date 2014
 * @copyright GNU Public License 2 or later
 * @see http://jimmieprodgers.com/kits/lolshield/
 *
 * This driver prepares the A0-A5 pins of an Ardunino device to serve as
 * joystick input lines.
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

#include <avr/io.h>

void joy_init(){
#if defined (__AVR_ATmega8__)    || \
    defined (__AVR_ATmega48__)   || \
    defined (__AVR_ATmega48P__)  || \
    defined (__AVR_ATmega88__)   || \
    defined (__AVR_ATmega88P__)  || \
    defined (__AVR_ATmega168__)  || \
    defined (__AVR_ATmega168P__) || \
    defined (__AVR_ATmega328__)  || \
    defined (__AVR_ATmega328P__)
	/*
	 * A0 => PC0 => JOYISUP
	 * A1 => PC1 => JOYISDOWN
	 * A2 => PC2 => JOYISLEFT
	 * A3 => PC3 => JOYISRIGHT
	 * A4 => PC4 => NC (JOYIS2GND)
	 * A5 => PC5 => JOYISFIRE
	 */

	// set joystick pins to input
	DDRC &= ~(_BV(PINC0) | _BV(PINC1) | _BV(PINC2) |
			  _BV(PINC3) | _BV(PINC4) | _BV(PINC5));
	// activate pullups
	PORTC |= _BV(PINC0) | _BV(PINC1) | _BV(PINC2) |
			 _BV(PINC3) | _BV(PINC4) | _BV(PINC5);
#elif defined (__AVR_ATmega32U4__)
	/*
	 * A0 => PF7 => JOYISUP
	 * A1 => PF6 => JOYISDOWN
	 * A2 => PF5 => JOYISLEFT
	 * A3 => PF4 => JOYISRIGHT
	 * A4 => PF1 => NC (JOYIS2GND)
	 * A5 => PF0 => JOYISFIRE
	 */

	// set joystick pins to input
	DDRF &= ~(_BV(PINF7) | _BV(PINF6) | _BV(PINF5) |
			  _BV(PINF4) | _BV(PINF1) | _BV(PINF0));
	// activate pullups
	PORTF |= _BV(PINF7) | _BV(PINF6) | _BV(PINF5) | _BV(PINF4) |
			 _BV(PINF1) | _BV(PINF0);
#elif defined (__AVR_ATmega1280__) || \
      defined (__AVR_ATmega2560__)
	/*
	 * A0 => PF0 => JOYISUP
	 * A1 => PF1 => JOYISDOWN
	 * A2 => PF2 => JOYISLEFT
	 * A3 => PF3 => JOYISRIGHT
	 * A4 => PF4 => NC (JOYIS2GND)
	 * A5 => PF5 => JOYISFIRE
	 */

	// set joystick pins to input
	DDRF &= ~(_BV(PINF0) | _BV(PINF1) | _BV(PINF2) |
			  _BV(PINF3) | _BV(PINF4) | _BV(PINF5));
	// activate pullups
	PORTF |= _BV(PINF0) | _BV(PINF1) | _BV(PINF2) |
			 _BV(PINF3) | _BV(PINF4) | _BV(PINF5);
#else
#	error "Unsupported Arduino board!"
#endif
}
