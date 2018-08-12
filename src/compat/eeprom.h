#ifndef EEROM_COMPAT_H
#define EEROM_COMPAT_H
//EEPPROM compatibility support for simulator

#include "../games/tetris/highscore_types.h"
#include "../random/persistentCounter.h"

enum eep {
	EEP_CAN_ADR     = 0,
	EEP_PER_COUNTER = 1,
	EEP_TETRIS      = (EEP_PER_COUNTER + sizeof(percnt_t)),
	EEP_END         = (EEP_TETRIS + sizeof(tetris_highscore_table_t))
};

#ifdef AVR
	#include <avr/eeprom.h>
#else
	#include <stdint.h>
	
	void eeprom_write_byte(uint8_t *p, uint8_t value);
	void eeprom_write_word(uint16_t *p, uint16_t value);
	
	uint8_t  eeprom_read_byte (const uint8_t *p);
	uint16_t eeprom_read_word (const uint16_t *p);
	
	#define eeprom_busy_wait()
	#define EEMEM
	
#endif

#endif // EEROM_COMPAT_H
