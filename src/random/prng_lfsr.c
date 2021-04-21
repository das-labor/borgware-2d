#include <stdint.h>

#include "prng.h"

uint16_t cnt16 = 91;
uint8_t random8(void) {
    return (uint8_t) ((cnt16 = (cnt16 >> 1) ^ (-(cnt16 & 1) & 0xB400)) & 0xFFFF);
}

void srandom32(uint32_t seed) {
    cnt16 = (uint16_t) (seed & 0xFFFF);
}