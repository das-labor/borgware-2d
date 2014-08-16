#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>
#include <avr/pgmspace.h>
#include "../config.h"
#include "../borg_hw/borg_hw.h"
#include "../scrolltext/scrolltext.h"
#include "uart.h"
#include "uart_commands.h"

#define UART_SCROLL_BUFFER_SIZE (SCROLLTEXT_BUFFER_SIZE + 8)
char g_rx_buffer[UART_SCROLL_BUFFER_SIZE];
uint8_t g_rx_index;

extern jmp_buf newmode_jmpbuf;
volatile unsigned char mode;

void uartcmd_clear_buffer(void) {
	char *p = &g_rx_buffer[0];
	for (uint8_t i = UART_SCROLL_BUFFER_SIZE; i--;) {
		*p++ = 0;
	}
	g_rx_index = 0;
}

char const UART_STR_PROMPT[]  PROGMEM = "\r\n> ";
char const UART_STR_ERROR[]   PROGMEM = "\r\ntransmission error";
char const UART_STR_UNKNOWN[] PROGMEM = "\r\nunknown command";
char const UART_STR_TOOLONG[] PROGMEM = "\r\ncommand to long";
char const UART_STR_HELP[]    PROGMEM = "\r\nhelp msg next prev reset scroll";

char const UART_CMD_HELP[]    PROGMEM = "help";
char const UART_CMD_MSG[]     PROGMEM = "msg ";
char const UART_CMD_NEXT[]    PROGMEM = "next";
char const UART_CMD_PREV[]    PROGMEM = "prev";
char const UART_CMD_RESET[]   PROGMEM = "reset";
char const UART_CMD_SCROLL[]  PROGMEM = "scroll ";

bool uartcmd_read_until_enter(void) {
	while (g_rx_index < (UART_SCROLL_BUFFER_SIZE - 1)) {
		int uart_result = uart_getc();

		switch (uart_result & 0xFF00u) {
		case 0:
			if (!(uart_result == '\r' || uart_result == '\n')) {
				g_rx_buffer[g_rx_index++] = uart_result;
				uart_putc(uart_result);
			} else {
				g_rx_buffer[g_rx_index++] = 0;
				return true;
			}
			break;

		case UART_FRAME_ERROR:
		case UART_OVERRUN_ERROR:
		case UART_PARITY_ERROR:
		case UART_BUFFER_OVERFLOW:
			uartcmd_clear_buffer();
			uart_puts_p(UART_STR_ERROR);
			uart_puts_p(UART_STR_PROMPT);
			break;

		case UART_NO_DATA:
		default:
			return false;
			break;
		}
	}

	if (g_rx_index >= (UART_SCROLL_BUFFER_SIZE - 1)) {
		uartcmd_clear_buffer();
		uart_puts_p(UART_STR_TOOLONG);
		uart_puts_p(UART_STR_PROMPT);
	}
	return false;
}

void uartcmd_process(void) {
	if (uartcmd_read_until_enter()) {
		if (!strncmp_P(g_rx_buffer, UART_CMD_HELP, 4)) {
			uart_puts_p(UART_STR_HELP);
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_MSG, 4)) {
			g_rx_buffer[1] = '<';
			g_rx_buffer[2] = '/';
			g_rx_buffer[3] = '#';
			// text must not be longer than the scroll text buffer
			g_rx_buffer[1 + SCROLLTEXT_BUFFER_SIZE - 1] = 0;
			scrolltext(&g_rx_buffer[1]);
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_NEXT, 4)) {
			uart_puts_p(UART_STR_PROMPT);
			uartcmd_clear_buffer();
			longjmp(newmode_jmpbuf, mode);
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_PREV, 4)) {
			uart_puts_p(UART_STR_PROMPT);
			uartcmd_clear_buffer();
			if (mode > 1) {
				longjmp(newmode_jmpbuf, mode - 2);
			}
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_RESET,
				UART_SCROLL_BUFFER_SIZE)) {
			timer0_off();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_SCROLL, 7)) {
			// text must not be longer than the scroll text buffer
			g_rx_buffer[7 + SCROLLTEXT_BUFFER_SIZE - 1] = 0;
			scrolltext(&g_rx_buffer[7]);
		} else {
			uart_puts_p(UART_STR_UNKNOWN);
		}
		uart_puts_p(UART_STR_PROMPT);
		uartcmd_clear_buffer();
	}
}
