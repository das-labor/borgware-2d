
#include <setjmp.h>
#include "compat/interrupt.h"
#include "compat/pgmspace.h"
#include <stdio.h>

#include "config.h"
#include "borg_hw/borg_hw.h"
// #include "can/borg_can.h"
#include "random/prng.h"
#include "random/persistentCounter.h"
#include "display_loop.h"
#include "pixel.h"

#ifdef JOYSTICK_SUPPORT
	#include "joystick/joystick.h"
#endif

#ifdef CAN_SUPPORT
	#include "can/borg_can.h"
#endif

#ifdef UART_SUPPORT
#    include "uart/uart.h"
#endif

#ifdef RFM12_SUPPORT
	#include "rfm12/borg_rfm12.h"
#endif


int main (void){
    puts("Program Start");
	clear_screen(0);

#ifdef RFM12_SUPPORT
	borg_rfm12_init();
#endif

    puts("HW Init");
	borg_hw_init();
    puts("HW Init Complete");

#ifdef CAN_SUPPORT
	bcan_init();
#endif

#ifdef UART_SUPPORT
	uart_init(UART_BAUD_SELECT(UART_BAUDRATE_SETTING, F_CPU));
#endif

#ifdef JOYSTICK_SUPPORT
    puts("JOYSTICK INIT");
	joy_init();	
#endif


	sei();
	display_loop();
	return 0;
}
