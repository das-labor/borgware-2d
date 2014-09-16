#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "../config.h"
#include "../borg_hw/borg_hw.h"
#ifdef JOYSTICK_SUPPORT
	#include "../joystick/joystick.h"
	extern unsigned char waitForFire;
#endif
#include "../scrolltext/scrolltext.h"
#include "uart.h"
#include "uart_commands.h"

#ifndef USE_UART1
#	define UART_PUTS_P(STR) uart_puts_p(STR)
#	define UART_GETC uart_getc
#else
#	define UART_PUTS_P(STR) uart1_puts_p(STR)
#	define UART_GETC uart1_getc
#endif

#define UART_BUFFER_SIZE (SCROLLTEXT_BUFFER_SIZE + 8)
char g_rx_buffer[UART_BUFFER_SIZE];
uint8_t g_rx_index;

extern jmp_buf newmode_jmpbuf;
extern volatile unsigned char mode;
extern volatile unsigned char reverseMode;

#if !(defined(eeprom_update_block) && \
	((E2PAGESIZE == 2) || (E2PAGESIZE == 4) || (E2PAGESIZE == 8)))
char const UART_STR_NOTIMPL[] PROGMEM = "\r\nnot implemented";
#endif

char const UART_STR_BACKSPACE[]  PROGMEM = "\033[D \033[D";
char const UART_STR_PROMPT[]     PROGMEM = "\r\n> ";
char const UART_STR_MODE[]       PROGMEM = "\r\n%d";
char const UART_STR_MODE_ERR[]   PROGMEM = "\r\nRange is between 0 and 255.";
char const UART_STR_GAMEMO_ERR[] PROGMEM = "\r\nNo mode change during games.";
char const UART_STR_GAMETX_ERR[] PROGMEM = "\r\nNo text messages during games.";
char const UART_STR_UART_ERR[]   PROGMEM = "\r\nTransmission error.";
char const UART_STR_UNKNOWN[]    PROGMEM = "\r\nUnknown command or syntax "
                                           "error.";
char const UART_STR_TOOLONG[]    PROGMEM = "\r\nCommand is to long.";
char const UART_STR_HELP[]       PROGMEM = "\r\nAllowed commands: erase help "
                                           "mode msg next prev reset scroll";

char const UART_CMD_ERASE[]      PROGMEM = "erase";
char const UART_CMD_HELP[]       PROGMEM = "help";
char const UART_CMD_MODE[]       PROGMEM = "mode";
char const UART_CMD_MODE_ARG[]   PROGMEM = "mode ";
char const UART_CMD_MSG[]        PROGMEM = "msg ";
char const UART_CMD_NEXT[]       PROGMEM = "next";
char const UART_CMD_PREV[]       PROGMEM = "prev";
char const UART_CMD_RESET[]      PROGMEM = "reset";
char const UART_CMD_SCROLL[]     PROGMEM = "scroll ";


bool g_uartcmd_permit_processing = 1;


/**
 * Checks if command processing is allowed.
 */
static bool uartcmd_processing_allowed(void) {
	bool result;
	cli();
	result = g_uartcmd_permit_processing;
	sei();
	return result;
}


/**
 * Clears the command string buffer.
 */
static void uartcmd_clear_buffer(void) {
	char *p = &g_rx_buffer[0];
	for (uint8_t i = UART_BUFFER_SIZE; i--;) {
		*p++ = 0;
	}
	g_rx_index = 0;
}


/**
 * Erases the complete EEPROM to reset counters and high score tables.
 */
static void uartcmd_erase_eeprom(void) {
#if defined(eeprom_update_block) && \
	((E2PAGESIZE == 2) || (E2PAGESIZE == 4) || (E2PAGESIZE == 8))
	uint8_t const eeclear[] =
#	if E2PAGESIZE == 8
		{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#	elif E2PAGESIZE == 4
		{0xFF, 0xFF, 0xFF, 0xFF};
#	elif E2PAGESIZE == 2
		{0xFF, 0xFF};
#	endif
	for (void *ee = 0; ee < (void *)E2END; ee += E2PAGESIZE) {
		eeprom_busy_wait();
		eeprom_update_block(eeclear, ee, E2PAGESIZE);
	}
#else
	UART_PUTS_P(UART_STR_NOTIMPL);
#endif
}


/**
 * Displays a simple message without the need to prefix a scrolltext command.
 */
static void uartcmd_simple_message(void) {
	if (uartcmd_processing_allowed()) {
		uartcmd_forbid();
#ifdef JOYSTICK_SUPPORT
		if (waitForFire) {
#else
		{
#endif
			g_rx_buffer[1] = '<';
			g_rx_buffer[2] = '/';
			g_rx_buffer[3] = '#';
			// text must not be longer than the scroll text buffer
			g_rx_buffer[1 + SCROLLTEXT_BUFFER_SIZE - 1] = 0;
			scrolltext(&g_rx_buffer[1]);
#ifdef JOYSTICK_SUPPORT
		} else {
#endif
			UART_PUTS_P(UART_STR_GAMETX_ERR);
		}
		uartcmd_permit();
	}
}


/**
 * Displays a message which may use the complete range of scrolltext commands.
 */
static void uartcmd_scroll_message(void) {
	if (uartcmd_processing_allowed()) {
		uartcmd_forbid();
#ifdef JOYSTICK_SUPPORT
		if (waitForFire) {
#else
		{
#endif
			// text must not be longer than the scroll text buffer
			g_rx_buffer[7 + SCROLLTEXT_BUFFER_SIZE - 1] = 0;
			scrolltext(&g_rx_buffer[7]);
#ifdef JOYSTICK_SUPPORT
		} else {
#endif
			UART_PUTS_P(UART_STR_GAMETX_ERR);
		}
		uartcmd_permit();
	}
}


/**
 * As long there's no game active, jump to the next animation.
 */
static void uartcmd_next_anim(void) {
#ifdef JOYSTICK_SUPPORT
	if (waitForFire) {
#endif
		UART_PUTS_P(UART_STR_PROMPT);
		uartcmd_clear_buffer();
		longjmp(newmode_jmpbuf, mode);
#ifdef JOYSTICK_SUPPORT
	} else {
		UART_PUTS_P(UART_STR_GAMEMO_ERR);
	}
#endif
}


/**
 * As long there's no game active, jump to the previous animation.
 */
static void uartcmd_prev_anim(void) {
#ifdef JOYSTICK_SUPPORT
	if (waitForFire) {
#endif
		reverseMode = mode - 2;
		UART_PUTS_P(UART_STR_PROMPT);
		uartcmd_clear_buffer();
		longjmp(newmode_jmpbuf, mode - 2);
#ifdef JOYSTICK_SUPPORT
	} else {
		UART_PUTS_P(UART_STR_GAMEMO_ERR);
	}
#endif
}


/**
 * Outputs current mode number via UART.
 */
static void uartcmd_print_mode(void) {
	char mode_output[6] = "";
	snprintf_P(mode_output, 6, UART_STR_MODE, mode - 1);
	uart_puts(mode_output);
}


/**
 * Retrieves desired mode number from command line and switches to that mode.
 */
static void uartcmd_read_mode(void) {
	int res = 0;
	for (uint8_t i = 5; (i < 8) && (g_rx_buffer[i] != 0); ++i) {
		if (isdigit(g_rx_buffer[i])) {
			res = res * 10 + g_rx_buffer[i] - '0';
		} else {
			res = UINT8_MAX + 1;
			break;
		}
	}

	if ((res <= UINT8_MAX) && (g_rx_buffer[8] == 0)) {
#ifdef JOYSTICK_SUPPORT
		if (waitForFire) {
#endif
			UART_PUTS_P(UART_STR_PROMPT);
			uartcmd_clear_buffer();
			longjmp(newmode_jmpbuf, res);
#ifdef JOYSTICK_SUPPORT
		} else {
			UART_PUTS_P(UART_STR_GAMEMO_ERR);
		}
#endif
	} else {
		UART_PUTS_P(UART_STR_MODE_ERR);
	}
}


/**
 * Perform a MCU reset by triggering the watchdog.
 */
static void uartcmd_reset_borg(void) {
	timer0_off();
}


/**
 * Appends new characters the buffer until a line break is entered.
 * @return true if a line break was entered, false otherwise.
 */
static bool uartcmd_read_until_enter(void) {
	while (g_rx_index < (UART_BUFFER_SIZE - 1)) {
		int uart_result = uart_getc();

		if (uart_result < 0x100u) {
			switch ((char)uart_result) {
			case '\r': // carriage return
			case '\n': // line feed
				if (g_rx_index != 0) {
					g_rx_buffer[g_rx_index++] = 0;
					return true;
				}
				break;
			case '\b':   // BS
			case '\177': // DEL
				if (g_rx_index != 0) {
					g_rx_buffer[--g_rx_index] = 0;
					UART_PUTS_P(UART_STR_BACKSPACE);
				}
				break;
			case 27: // ignore Esc
				break;
			default:
				g_rx_buffer[g_rx_index++] = uart_result;
				uart_putc(uart_result);
				break;
			}
		} else if ((uart_result & 0xFF00u) != UART_NO_DATA) {
			uartcmd_clear_buffer();
			UART_PUTS_P(UART_STR_UART_ERR);
			UART_PUTS_P(UART_STR_PROMPT);
			break;
		} else {
			break;
		}
	}

	if (g_rx_index >= (UART_BUFFER_SIZE - 1)) {
		uartcmd_clear_buffer();
		UART_PUTS_P(UART_STR_TOOLONG);
		UART_PUTS_P(UART_STR_PROMPT);
	}
	return false;
}

/**
 * Checks for entered commands and dispatches them to the appropriate handler.
 */
void uartcmd_process(void) {
	if (uartcmd_processing_allowed() && uartcmd_read_until_enter()) {
		if (!strncmp_P(g_rx_buffer, UART_CMD_ERASE, UART_BUFFER_SIZE)) {
			uartcmd_erase_eeprom();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_HELP, UART_BUFFER_SIZE)) {
			UART_PUTS_P(UART_STR_HELP);
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_MODE, UART_BUFFER_SIZE) ||
				!strncmp_P(g_rx_buffer, UART_CMD_MODE_ARG, UART_BUFFER_SIZE)) {
			uartcmd_print_mode();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_MODE_ARG, 5)) {
			uartcmd_read_mode();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_MSG, 4)) {
			uartcmd_simple_message();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_NEXT, UART_BUFFER_SIZE)) {
			uartcmd_next_anim();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_PREV, UART_BUFFER_SIZE)) {
			uartcmd_prev_anim();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_RESET, UART_BUFFER_SIZE)) {
			uartcmd_reset_borg();
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_SCROLL, 7)) {
			uartcmd_scroll_message();
		} else {
			UART_PUTS_P(UART_STR_UNKNOWN);
		}
		UART_PUTS_P(UART_STR_PROMPT);
		uartcmd_clear_buffer();
	}
}
