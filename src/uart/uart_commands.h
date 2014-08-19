/*
 * uart_commands.h
 *
 *  Created on: 16.08.2014
 *      Author: chris
 */

#ifndef UART_COMMANDS_H_
#define UART_COMMANDS_H_

#include <stdbool.h>
#include <avr/interrupt.h>

extern bool g_uartcmd_permit_processing;

/**
 * Enables UART command processing.
 */
inline static void uartcmd_permit(void) {
	cli();
	g_uartcmd_permit_processing = true;
	sei();
}


/**
 * Disables UART command processing.
 */
inline static void uartcmd_forbid(void) {
	cli();
	g_uartcmd_permit_processing = false;
	sei();
}


void uartcmd_process(void);

#endif /* UART_COMMANDS_H_ */
