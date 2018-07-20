/**
 * \file	persistentCounter.h
 * \author	Daniel Otte 
 * \brief	a persistent 24-bit vounter in EEPROM for ATmega µC
 * 
 * 
 */

#ifndef PERSISTENTCOUNTER_H_
#define PERSISTENTCOUNTER_H_

#include <stdint.h>
#include "../compat/eeprom.h"

#define PERSISTENT_COUNTER_BITS 24
#define RING_SIZE 168

typedef struct percnt_s {
	uint16_t B08_23;
	uint8_t  B0_7[RING_SIZE];
} percnt_t;

extern uint8_t  g_reset_counter_idx;
//extern percnt_t g_reset_counter EEMEM;

void percnt_init(percnt_t *percnt, uint8_t *ring_index);
uint32_t percnt_get(percnt_t *percnt, uint8_t *ring_index);
void percnt_inc(percnt_t *percnt, uint8_t *ring_index);

#endif /*PERSISTENTCOUNTER_H_*/
