#ifndef JOYSTICK_H
#define JOYSTICK_H

#ifdef __AVR__
#	include <avr/io.h>
#endif

#include "../config.h"

extern unsigned char waitForFire;
void joy_init();

#ifdef BROWSER_JOYSTICK_SUPPORT
	enum JoystickButton {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		FIRE
	};

	int joy_button_pressed(enum JoystickButton button);

#	define JOYISUP    (joy_button_pressed(UP))
#	define JOYISDOWN  (joy_button_pressed(DOWN))
#	define JOYISLEFT  (joy_button_pressed(LEFT))
#	define JOYISRIGHT (joy_button_pressed(RIGHT))
#	define JOYISFIRE  (joy_button_pressed(FIRE))

#elif defined(AVR)

#	include <avr/io.h>
#	ifdef NES_PAD_SUPPORT

		extern volatile unsigned char fakeport;

#		define JOYISUP    (!! ((1<<4) & fakeport))
#		define JOYISDOWN  (!! ((1<<5) & fakeport))
#		define JOYISLEFT  (!! ((1<<6) & fakeport))
#		define JOYISRIGHT (!! ((1<<7) & fakeport))
#		define JOYISFIRE  (!! ((1<<0) & fakeport))

#	elif defined(RFM12_JOYSTICK_SUPPORT)

		extern volatile uint8_t rfm12_joystick_val;

#		define JOYISUP    (!! ((1<<0) & rfm12_joystick_val))
#		define JOYISDOWN  (!! ((1<<1) & rfm12_joystick_val))
#		define JOYISLEFT  (!! ((1<<2) & rfm12_joystick_val))
#		define JOYISRIGHT (!! ((1<<3) & rfm12_joystick_val))
#		define JOYISFIRE  (!! ((1<<4) & rfm12_joystick_val))

#	elif defined(HC165_JOYSTICK_SUPPORT)

		extern volatile uint8_t hc165_joystick_val;

#		define JOYISUP    (! ((1<<0) & hc165_joystick_val))
#		define JOYISDOWN  (! ((1<<1) & hc165_joystick_val))
#		define JOYISLEFT  (! ((1<<2) & hc165_joystick_val))
#		define JOYISRIGHT (! ((1<<3) & hc165_joystick_val))
#		define JOYISFIRE  (! ((1<<4) & hc165_joystick_val))


#	elif defined(NULL_JOYSTICK_SUPPORT)

#		define JOYISUP    (0)
#		define JOYISDOWN  (0)
#		define JOYISLEFT  (0)
#		define JOYISRIGHT (0)
#		define JOYISFIRE  (0)

#	elif defined (LOLSHIELD_JOYSTICK_SUPPORT)

#		if defined (__AVR_ATmega8__)    || \
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
#			define JOYISUP    (!(PINC & _BV(PINC0)))
#			define JOYISDOWN  (!(PINC & _BV(PINC1)))
#			define JOYISLEFT  (!(PINC & _BV(PINC2)))
#			define JOYISRIGHT (!(PINC & _BV(PINC3)))
#			define JOYISFIRE  (!(PINC & _BV(PINC5)))
#		elif defined (__AVR_ATmega32U4__)
			/*
			 * A0 => PF7 => JOYISUP
			 * A1 => PF6 => JOYISDOWN
			 * A2 => PF5 => JOYISLEFT
			 * A3 => PF4 => JOYISRIGHT
			 * A4 => PF1 => NC (JOYIS2GND)
			 * A5 => PF0 => JOYISFIRE
			 */
#			define JOYISUP    (!(PINF & _BV(PINF7)))
#			define JOYISDOWN  (!(PINF & _BV(PINF6)))
#			define JOYISLEFT  (!(PINF & _BV(PINF5)))
#			define JOYISRIGHT (!(PINF & _BV(PINF4)))
#			define JOYISFIRE  (!(PINF & _BV(PINF0)))
#		elif defined (__AVR_ATmega1280__) || \
			 defined (__AVR_ATmega2560__)
			/*
			 * A0 => PF0 => JOYISUP
			 * A1 => PF1 => JOYISDOWN
			 * A2 => PF2 => JOYISLEFT
			 * A3 => PF3 => JOYISRIGHT
			 * A4 => PF4 => NC (JOYIS2GND)
			 * A5 => PF5 => JOYISFIRE
			 */
#			define JOYISUP    (!(PINF & _BV(PINF0)))
#			define JOYISDOWN  (!(PINF & _BV(PINF1)))
#			define JOYISLEFT  (!(PINF & _BV(PINF2)))
#			define JOYISRIGHT (!(PINF & _BV(PINF3)))
#			define JOYISFIRE  (!(PINF & _BV(PINF5)))
#		else
#			error "Unsupported Arduino board!"
#		endif

#	elif defined (PARALLEL_JOYSTICK_SUPPORT)

#		define JOYISUP    (!(JOYSTICK_PIN_UP    & (1<<JOYSTICK_BIT_UP   )))
#		define JOYISDOWN  (!(JOYSTICK_PIN_DOWN  & (1<<JOYSTICK_BIT_DOWN )))
#		define JOYISLEFT  (!(JOYSTICK_PIN_LEFT  & (1<<JOYSTICK_BIT_LEFT )))
#		define JOYISRIGHT (!(JOYSTICK_PIN_RIGHT & (1<<JOYSTICK_BIT_RIGHT)))
#		define JOYISFIRE  (!(JOYSTICK_PIN_FIRE  & (1<<JOYSTICK_BIT_FIRE )))

#	endif
#else

	extern volatile unsigned char fakeport;

#	define JOYISUP    (0x10 & fakeport)
#	define JOYISDOWN  (0x08 & fakeport)    
#	define JOYISLEFT  (0x02 & fakeport)
#	define JOYISRIGHT (0x04 & fakeport)
#	define JOYISFIRE  (0x01 & fakeport)

#endif

#endif // JOYSTICK_H
