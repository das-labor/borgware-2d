#include "config.h"
#include <stdio.h>

//#include <avr/io.h>
#include <setjmp.h>

#ifdef JOYSTICK_SUPPORT
#  include "joystick/joystick.h"
	unsigned char waitForFire;
#endif

#ifdef RFM12_SUPPORT
	#include "rfm12/borg_rfm12.h"
#endif

//this buffer is declared in main
#include "display_loop.h"

#ifdef CAN_SUPPORT
#  include "can/borg_can.h"
#endif

#ifdef UART_SUPPORT
#  include "uart/uart_commands.h"
#endif

#include "platform/platform.h"

void b2d_wait(int ms) {

	for(;ms>0;ms--){

#ifdef CAN_SUPPORT
		bcan_process_messages();
#endif

#ifdef UART_SUPPORT
		uartcmd_process();
#endif

#ifdef RFM12_SUPPORT
		borg_rfm12_tick();
#endif

#ifdef JOYSTICK_SUPPORT
		if (waitForFire) {
			//PORTJOYGND &= ~(1<<BITJOY0);
			//PORTJOYGND &= ~(1<<BITJOY1);
			if (JOYISFIRE) {
				longjmp(newmode_jmpbuf, 0xFEu);
			}
			if (JOYISRIGHT) {
				waitForFire = 0;
				while (JOYISRIGHT) {
					b2d_wait(5);
				}
				waitForFire = 1;
				longjmp(newmode_jmpbuf, mode + 1);
			}
		}
#endif

		sleep_step();
	}
}
