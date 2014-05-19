#include "config.h"

#include <avr/io.h>
#include <setjmp.h>

#ifdef JOYSTICK_SUPPORT
#  include "joystick/joystick.h"
	unsigned char waitForFire;
#endif

#ifdef RFM12_SUPPORT
	#include "rfm12/borg_rfm12.h"
#endif

//this buffer is declared in main
extern jmp_buf newmode_jmpbuf;

#ifdef CAN_SUPPORT
#  include "can/borg_can.h"
#endif


void wait(int ms){
/* Always use Timer1 except for the Arduino/LoL Shield platform. */
#ifndef USER_TIMER0_FOR_WAIT
	TCCR1B = _BV(WGM12) | _BV(CS12); //CTC Mode, clk/256
	OCR1A = (F_CPU/256000);	         //1000Hz
/* Some Arduino/LoL Shield variants require Timer1 for multiplexing. Timer0, on
 * the other hand, is free to use, which makes it a perfect candidate for our
 * wait() function. */
#else
	// disconnect OC0A and OC0B pins, turn on CTC mode, clk/256
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS02);
	OCR0A = (F_CPU/256000);	//1000Hz
#endif

	for(;ms>0;ms--){

#ifdef CAN_SUPPORT
		bcan_process_messages();
#endif

#ifdef RFM12_SUPPORT
		borg_rfm12_tick();
#endif

#ifdef JOYSTICK_SUPPORT
		if (waitForFire) {
			//PORTJOYGND &= ~(1<<BITJOY0);
			//PORTJOYGND &= ~(1<<BITJOY1);
			if (JOYISFIRE) {
				longjmp(newmode_jmpbuf, 43);
			}
		}
#endif

#if defined (__AVR_ATmega32U4__) || defined (__AVR_ATmega644P__) || defined (__AVR_ATmega644__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1284P__) || defined (__AVR_ATmega1284__)
/* Timer1 for the masses */
#	ifndef USER_TIMER0_FOR_WAIT
		while(!(TIFR1 & _BV(OCF1A))); //wait for compare match flag
			TIFR1 |= _BV(OCF1A);      //reset flag
/* Timer0 for e.g. Arduino/LoL Shield */
#	else
		while(!(TIFR0 & _BV(OCF0A))); //wait for compare match flag
			TIFR0 |= _BV(OCF0A);      //reset flag
#	endif
#else
		while(!(TIFR&(1<<OCF1A)));	//wait for compare match flag
		TIFR=(1<<OCF1A);		//reset flag
#endif
	}
}
