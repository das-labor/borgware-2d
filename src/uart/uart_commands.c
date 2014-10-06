#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>
#include <limits.h>
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
#include "../animations/program.h"
#include "uart.h"
#include "uart_commands.h"

#ifndef USE_UART1
#	define UART_PUTS(STR) uart_puts(STR)
#	define UART_PUTS_P(STR) uart_puts_p(STR)
#	define UART_GETC uart_getc
#	define UART_PUTC uart_putc
#else
#	define UART_PUTS(STR) uart1_puts(STR)
#	define UART_PUTS_P(STR) uart1_puts_p(STR)
#	define UART_GETC uart1_getc
#	define UART_PUTC uart1_putc
#endif

#define UART_BUFFER_SIZE (SCROLLTEXT_BUFFER_SIZE + 8)
char g_rx_buffer[UART_BUFFER_SIZE];
uint8_t g_rx_index;

extern jmp_buf newmode_jmpbuf;
extern volatile unsigned char mode;
extern volatile unsigned char reverseMode;

#define CR "\r\n"

#if (!(defined(eeprom_update_block) && \
	((E2PAGESIZE == 2) || (E2PAGESIZE == 4) || (E2PAGESIZE == 8)))) || \
	!(defined(ANIMATION_TESTS))
char const UART_STR_NOTIMPL[] PROGMEM = "Not implemented."CR;
#endif

char const UART_STR_CLEARLINE[]  PROGMEM = "\r\033[J";
char const UART_STR_BACKSPACE[]  PROGMEM = "\b \b";
char const UART_STR_PROMPT[]     PROGMEM = "> ";
char const UART_STR_MODE[]       PROGMEM = "%d"CR;
char const UART_STR_MODE_ERR[]   PROGMEM = "Range is between 0 and 255."CR;
char const UART_STR_GAMEMO_ERR[] PROGMEM = "No mode change during games."CR;
char const UART_STR_GAMETX_ERR[] PROGMEM = "No text messages during games."CR;
char const UART_STR_UART_ERR[]   PROGMEM = "Transmission error."CR;
char const UART_STR_UNKNOWN[]    PROGMEM = "Unknown command or syntax error."CR;
char const UART_STR_TOOLONG[]    PROGMEM = CR"Command is too long."CR;
char const UART_STR_HELP[]       PROGMEM = "Allowed commands: erase help mode "
                                           "msg next prev reset scroll test"CR;

char const UART_CMD_ERASE[]      PROGMEM = "erase";
char const UART_CMD_HELP[]       PROGMEM = "help";
char const UART_CMD_MODE[]       PROGMEM = "mode";
char const UART_CMD_MSG[]        PROGMEM = "msg ";
char const UART_CMD_NEXT[]       PROGMEM = "next";
char const UART_CMD_PREV[]       PROGMEM = "prev";
char const UART_CMD_RESET[]      PROGMEM = "reset";
char const UART_CMD_SCROLL[]     PROGMEM = "scroll ";
char const UART_CMD_TEST[]       PROGMEM = "test";

#ifdef ANIMATION_TESTS
char const UART_STR_TEST_EXIT[]  PROGMEM = "Press ENTER to exit test."CR;
char const UART_STR_TEST_ERR[]   PROGMEM = "Range is between 0 and %d."CR;
char const UART_STR_GAMETS_ERR[] PROGMEM = "No display tests during games."CR;
char const UART_STR_TEST_UP[]    PROGMEM = "UP ";
char const UART_STR_TEST_DOWN[]  PROGMEM = "DOWN ";
char const UART_STR_TEST_LEFT[]  PROGMEM = "LEFT ";
char const UART_STR_TEST_RIGHT[] PROGMEM = "RIGHT ";
char const UART_STR_TEST_FIRE[]  PROGMEM = "FIRE";

enum uartcmd_joytest_e {
	UARTCMD_JOY_UP    = 0x01,
	UARTCMD_JOY_DOWN  = 0x02,
	UARTCMD_JOY_LEFT  = 0x04,
	UARTCMD_JOY_RIGHT = 0x08,
	UARTCMD_JOY_FIRE  = 0x10
};

#	ifdef NDEBUG
	typedef uint8_t uartcmd_joytest_t;
#	else
	typedef enum uartcmd_joytest_e uartcmd_joytest_t;
#	endif
#endif


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
			waitForFire = 0;
#endif
			g_rx_buffer[1] = '<';
			g_rx_buffer[2] = '/';
			g_rx_buffer[3] = '#';
			// text must not be longer than the scroll text buffer
			g_rx_buffer[1 + SCROLLTEXT_BUFFER_SIZE - 1] = 0;
			scrolltext(&g_rx_buffer[1]);

#ifdef JOYSTICK_SUPPORT
			waitForFire = 1;
		} else {
			UART_PUTS_P(UART_STR_GAMETX_ERR);
		}
#endif
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
			waitForFire = 0;
#endif
			// text must not be longer than the scroll text buffer
			g_rx_buffer[7 + SCROLLTEXT_BUFFER_SIZE - 1] = 0;
			scrolltext(&g_rx_buffer[7]);
#ifdef JOYSTICK_SUPPORT
			waitForFire = 1;
		} else {
			UART_PUTS_P(UART_STR_GAMETX_ERR);
		}
#endif
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
 * Extracts a positive number from an ASCII string (up to INT_MAX).
 * @param buffer String to be examined. Preceding whitespaces are skipped.
 *               ASCII control characters and [Space] act as delimiters.
 * @return The extracted number or -1 if a conversion error occurred.
 */
int uartcmd_extract_num_arg(char *buffer) {
	int res = -1;
	uint8_t i = 0;
	while (buffer[i] <= ' ' && buffer[i] != 0) ++i; // skip whitespaces
	for (; buffer[i] > 0x20; ++i) {
		if (res < 0) {
			res = 0;
		}
		int const d = buffer[i] - '0';
		if (isdigit(buffer[i]) && (res <= (INT_MAX / 10)) &&
				((res * 10) <= (INT_MAX - d))) {
			res = res * 10 + d;
		} else {
			res = -1;
			break;
		}
	}
	return res;
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
static void uartcmd_mode(void) {
	if (g_rx_buffer[4] != 0) {
		int new_mode = uartcmd_extract_num_arg(&g_rx_buffer[4]);
		if (new_mode <= UINT8_MAX) {
#ifdef JOYSTICK_SUPPORT
			if (waitForFire) {
#endif
				UART_PUTS_P(UART_STR_PROMPT);
				uartcmd_clear_buffer();
				longjmp(newmode_jmpbuf, new_mode);
#ifdef JOYSTICK_SUPPORT
			} else {
				UART_PUTS_P(UART_STR_GAMEMO_ERR);
			}
#endif
		} else {
			UART_PUTS_P(UART_STR_MODE_ERR);
		}
	} else {
		uartcmd_print_mode();
	}
}


#if defined(ANIMATION_TESTS) && defined(JOYSTICK_SUPPORT)
/**
 * Prints current joystick status via UART.
 */
static void uartcmd_joy_test(uint8_t *last) {
	uint8_t joy_value = 0;
	if (JOYISUP) {
		joy_value |= UARTCMD_JOY_UP;
	}
	if (JOYISDOWN) {
		joy_value |= UARTCMD_JOY_DOWN;
	}
	if (JOYISLEFT) {
		joy_value |= UARTCMD_JOY_LEFT;
	}
	if (JOYISRIGHT) {
		joy_value |= UARTCMD_JOY_RIGHT;
	}
	if (JOYISFIRE) {
		joy_value |= UARTCMD_JOY_FIRE;
	}

	if (joy_value != *last) {
		UART_PUTS_P(UART_STR_CLEARLINE);
		if (joy_value & UARTCMD_JOY_UP) {
			UART_PUTS_P(UART_STR_TEST_UP);
		}
		if (joy_value & UARTCMD_JOY_DOWN) {
			UART_PUTS_P(UART_STR_TEST_DOWN);
		}
		if (joy_value & UARTCMD_JOY_LEFT) {
			UART_PUTS_P(UART_STR_TEST_LEFT);
		}
		if (joy_value & UARTCMD_JOY_RIGHT) {
			UART_PUTS_P(UART_STR_TEST_RIGHT);
		}
		if (joy_value & UARTCMD_JOY_FIRE) {
			UART_PUTS_P(UART_STR_TEST_FIRE);
		}
		*last = joy_value;
	}
}
#endif


/**
 * Draws test patterns for display inspection.
 */
static void uartcmd_test(void) {
#ifdef ANIMATION_TESTS
#	ifdef JOYSTICK_SUPPORT
	if (waitForFire) {
		waitForFire = 0;
#	endif
		uartcmd_forbid();
		int pattern_no = uartcmd_extract_num_arg(&g_rx_buffer[4]);
		if (pattern_no >= 0 && pattern_no <= NUMPLANE + 2) {
			UART_PUTS_P(UART_STR_TEST_EXIT);
			if (pattern_no <= NUMPLANE) {
				test_level(pattern_no, true);
			} else if (pattern_no == (NUMPLANE + 1)) {
				test_palette(true);
			} else if (pattern_no == (NUMPLANE + 2)) {
				test_palette2(true);
			}
#	ifdef JOYSTICK_SUPPORT
			uint8_t last_joy_value = 0;
#	endif
			while (UART_GETC() >= 0x20){ // wait for any control character
#	ifdef JOYSTICK_SUPPORT
				uartcmd_joy_test(&last_joy_value);
				wait(20);
#	endif
			}
		} else {
			char msg[36] = "";
			snprintf_P(msg, sizeof(msg), UART_STR_TEST_ERR, NUMPLANE + 2);
			UART_PUTS(msg);
		}
		uartcmd_permit();
#	ifdef JOYSTICK_SUPPORT
		waitForFire = 1;
	} else {
		UART_PUTS_P(UART_STR_GAMETS_ERR);
	}
#	endif
#else
	UART_PUTS_P(UART_STR_NOTIMPL);
#endif
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
	static char last_line_break = '\n';

	while (g_rx_index < (UART_BUFFER_SIZE - 1)) {
		int uart_result = uart_getc();

		if (uart_result < 0x100u) {
			switch ((char)uart_result) {
			case '\n': // line feed
			case '\r': // carriage return
				if ((g_rx_index > 0) || (uart_result == last_line_break)) {
					UART_PUTS(CR);
					g_rx_buffer[g_rx_index++] = 0;
					last_line_break = uart_result;
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
			case '\f': // Form Feed (Ctrl-L), reprints the line buffer
				UART_PUTS_P(UART_STR_CLEARLINE); // clear current line
				UART_PUTS_P(UART_STR_PROMPT);    // output prompt
				g_rx_buffer[g_rx_index] = 0;     // terminate input buffer
				UART_PUTS(g_rx_buffer);          // finally reprint it
				break;
			case 21: // NAK (Ctrl-U), clears the line buffer
				UART_PUTS_P(UART_STR_CLEARLINE); // clear current line
				UART_PUTS_P(UART_STR_PROMPT);    // output prompt
				uartcmd_clear_buffer();          // clear buffer
				break;
			case 27: // ignore Esc
				break;
			default:
				// We don't accept control characters which are not handled
				// above. We also limit the input to 7 bit ASCII.
				if ((uart_result < 0x20) || (uart_result > 0x7f)) {
					UART_PUTC('\a'); // complain via ASCII bell
				} else {
					g_rx_buffer[g_rx_index++] = uart_result; // accept input
					UART_PUTC(uart_result); // echo input back to terminal
				}
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
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_MODE, 4)) {
			uartcmd_mode();
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
		} else if (!strncmp_P(g_rx_buffer, UART_CMD_TEST, 4)) {
			uartcmd_test();
		} else if (g_rx_buffer[0] != 0) {
			UART_PUTS_P(UART_STR_UNKNOWN);
		}
		UART_PUTS_P(UART_STR_PROMPT);
		uartcmd_clear_buffer();
	}
}
