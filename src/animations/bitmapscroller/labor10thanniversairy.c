#include <stdint.h>
#include <assert.h>

#include "../../compat/pgmspace.h"
#include "bitmapscroller.h"
#include "labor10thanniversairy.h"


static uint8_t const aLabor10thAnniversairyBitmap[2][89][8] PROGMEM =
	{{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x0F, 0x40, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0xAB, 0xB4, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x39, 0x3C, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x3B, 0x98, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x01, 0xE7, 0xCF, 0x00, 0x10, 0x00},
	  {0x00, 0x00, 0x01, 0xF3, 0xDF, 0x00, 0x10, 0x00},
	  {0x00, 0x00, 0x00, 0x2B, 0xD0, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x31, 0x18, 0x00, 0x40, 0x00},
	  {0x00, 0x01, 0x40, 0x3E, 0xA6, 0x00, 0xC0, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00},
	  {0x00, 0x00, 0x70, 0x00, 0x30, 0x01, 0x00, 0x00},
	  {0x00, 0x00, 0x50, 0x18, 0x00, 0x02, 0x00, 0x00},
	  {0x00, 0x00, 0x18, 0x00, 0x00, 0x0E, 0x00, 0x00},
	  {0x00, 0x00, 0x04, 0x00, 0x00, 0x1E, 0x80, 0x00},
	  {0x00, 0x02, 0x16, 0x00, 0x10, 0x1C, 0x00, 0x00},
	  {0x00, 0x03, 0x9C, 0x42, 0x00, 0x18, 0x80, 0x00},
	  {0x00, 0x03, 0x20, 0x8C, 0x00, 0x30, 0x00, 0x00},
	  {0x00, 0x01, 0xE0, 0x92, 0x10, 0x71, 0x00, 0x00},
	  {0x00, 0x20, 0x07, 0xC0, 0x30, 0xE7, 0x00, 0x00},
	  {0x00, 0x20, 0x23, 0xD2, 0x01, 0xE6, 0x00, 0x00},
	  {0x00, 0x10, 0x03, 0x20, 0x07, 0xC3, 0x80, 0x00},
	  {0x00, 0x00, 0x81, 0x60, 0x47, 0x89, 0xD4, 0x00},
	  {0x00, 0x1C, 0x80, 0x90, 0x4F, 0x00, 0xC0, 0x00},
	  {0x00, 0x0B, 0xF0, 0x18, 0x07, 0x25, 0xC0, 0x00},
	  {0x00, 0x07, 0xC8, 0x70, 0x3C, 0x8C, 0xA0, 0x00},
	  {0x00, 0x2F, 0xC4, 0x61, 0x20, 0x02, 0x70, 0x00},
	  {0x00, 0x07, 0xC0, 0x00, 0x08, 0x82, 0xD2, 0x00},
	  {0x00, 0x04, 0xFC, 0x20, 0x63, 0x9B, 0x0C, 0x00},
	  {0x00, 0x40, 0x7C, 0x01, 0x14, 0x01, 0x58, 0x00},
	  {0x00, 0x0A, 0x7F, 0x45, 0x41, 0xC8, 0xC0, 0x00},
	  {0x00, 0x01, 0x60, 0xDF, 0xF3, 0xE0, 0xC0, 0x00},
	  {0x00, 0x00, 0x38, 0x3F, 0xFF, 0x94, 0xC0, 0x00},
	  {0x00, 0x01, 0x7C, 0x1F, 0xFF, 0xFD, 0x00, 0x00},
	  {0x00, 0x02, 0xF8, 0x0F, 0xC3, 0x3F, 0xC0, 0x00},
	  {0x00, 0x00, 0x70, 0x0F, 0xCF, 0x80, 0xC0, 0x00},
	  {0x00, 0x00, 0x60, 0x07, 0xCF, 0x88, 0x40, 0x00},
	  {0x00, 0x00, 0x41, 0x03, 0xC3, 0x86, 0x40, 0x00},
	  {0x00, 0x00, 0x43, 0x07, 0xCF, 0x9E, 0x40, 0x00},
	  {0x00, 0x00, 0x63, 0x07, 0xDF, 0x86, 0x40, 0x00},
	  {0x00, 0x00, 0x63, 0x07, 0xC7, 0x86, 0x40, 0x00},
	  {0x00, 0x00, 0x63, 0x07, 0xC3, 0x86, 0x40, 0x00},
	  {0x00, 0x00, 0x63, 0x07, 0xFF, 0xDB, 0xC0, 0x00},
	  {0x00, 0x00, 0x63, 0x07, 0xFF, 0xFF, 0xC0, 0x00},
	  {0x00, 0x00, 0x43, 0x03, 0xC3, 0xC6, 0x40, 0x00},
	  {0x00, 0x00, 0x41, 0x87, 0xC0, 0x84, 0x40, 0x00},
	  {0x00, 0x00, 0x60, 0x07, 0xC9, 0x85, 0xC0, 0x00},
	  {0x00, 0x00, 0x60, 0x0F, 0xC5, 0x80, 0x40, 0x00},
	  {0x00, 0x00, 0x78, 0x1F, 0xC5, 0xAC, 0x40, 0x00},
	  {0x00, 0x02, 0xFF, 0xBF, 0xC5, 0xAC, 0xC0, 0x00},
	  {0x00, 0x00, 0x0C, 0x7F, 0xC5, 0x0C, 0xC0, 0x00},
	  {0x00, 0x03, 0x8E, 0x07, 0xC5, 0x26, 0xC0, 0x00},
	  {0x00, 0x02, 0x3B, 0x67, 0xC5, 0x86, 0xC0, 0x00},
	  {0x00, 0x02, 0xB6, 0x07, 0xC4, 0xA6, 0xC0, 0x00},
	  {0x00, 0x03, 0xA6, 0xA7, 0xC8, 0xA7, 0xC0, 0x00},
	  {0x00, 0x03, 0xFC, 0x6F, 0xC1, 0x17, 0xE0, 0x00},
	  {0x00, 0x02, 0xF8, 0xB7, 0xC5, 0xFE, 0x80, 0x00},
	  {0x00, 0x00, 0x37, 0x8F, 0xDF, 0xC8, 0x00, 0x00},
	  {0x00, 0x00, 0x01, 0xBF, 0xFD, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x0B, 0xD0, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x03, 0xE5, 0x69, 0x40, 0x00, 0x00},
	  {0x3F, 0xFF, 0xF4, 0xA7, 0x4D, 0x5F, 0xFF, 0xFC},
	  {0x00, 0x00, 0x03, 0x64, 0x75, 0xC0, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x3E, 0x3C, 0x7C, 0x10, 0x18, 0xFC, 0xF0, 0xF8},
	  {0x3E, 0x1C, 0xEC, 0x10, 0x3C, 0xED, 0xFC, 0xDC},
	  {0x33, 0xB4, 0x40, 0x10, 0x6C, 0xEC, 0x8E, 0x9C},
	  {0x31, 0xAE, 0xF0, 0x10, 0x34, 0xF9, 0xC4, 0xA8},
	  {0x31, 0x6E, 0x7C, 0x10, 0x76, 0xFD, 0xC4, 0xF8},
	  {0x33, 0xFE, 0x1E, 0x10, 0x7E, 0xEC, 0x8E, 0x90},
	  {0x37, 0x67, 0xDE, 0x1D, 0xE6, 0xDD, 0xEC, 0xD8},
	  {0x3E, 0x42, 0xF8, 0x1E, 0xE7, 0xF8, 0x7C, 0x8C},
	  {0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x3F, 0xED, 0x80, 0xA5, 0x5B, 0x09, 0x77, 0xF8},
	  {0x00, 0x0D, 0x02, 0xB3, 0xCB, 0x35, 0x50, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

	 {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x45, 0x42, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x08, 0x00},
	  {0x00, 0x00, 0x00, 0x1F, 0xF0, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x17, 0xE8, 0x00, 0x20, 0x00},
	  {0x00, 0x00, 0x60, 0x0F, 0xE0, 0x00, 0x40, 0x00},
	  {0x00, 0x00, 0xC0, 0x41, 0x50, 0x00, 0x80, 0x00},
	  {0x00, 0x00, 0xB0, 0x05, 0x00, 0x00, 0x80, 0x00},
	  {0x00, 0x00, 0x50, 0x09, 0x00, 0x01, 0x00, 0x00},
	  {0x00, 0x00, 0x68, 0x01, 0x10, 0x02, 0x00, 0x00},
	  {0x00, 0x00, 0x14, 0x01, 0x00, 0x04, 0x00, 0x00},
	  {0x00, 0x00, 0x38, 0x01, 0x00, 0x0C, 0x00, 0x00},
	  {0x00, 0x00, 0x02, 0x03, 0x00, 0x1C, 0x00, 0x00},
	  {0x00, 0x02, 0x0C, 0x01, 0x00, 0x38, 0x00, 0x00},
	  {0x00, 0x01, 0x8B, 0x01, 0x00, 0x73, 0x00, 0x00},
	  {0x00, 0x00, 0xC5, 0x0D, 0x00, 0xE3, 0x00, 0x00},
	  {0x00, 0x00, 0xE2, 0x13, 0x11, 0xE2, 0x00, 0x00},
	  {0x00, 0x10, 0x00, 0x8D, 0x03, 0xC6, 0x00, 0x00},
	  {0x00, 0x10, 0x01, 0x41, 0x03, 0xC4, 0x80, 0x00},
	  {0x00, 0x18, 0x00, 0xA1, 0x07, 0xC0, 0xB0, 0x00},
	  {0x00, 0x09, 0x80, 0xE1, 0x47, 0x80, 0x70, 0x00},
	  {0x00, 0x0D, 0xA0, 0x11, 0x0F, 0x00, 0x80, 0x00},
	  {0x00, 0x0F, 0xC0, 0x61, 0x06, 0x44, 0x80, 0x00},
	  {0x00, 0x27, 0xC0, 0x03, 0x00, 0x04, 0xE0, 0x00},
	  {0x00, 0x06, 0xEC, 0x01, 0x00, 0x07, 0x96, 0x00},
	  {0x00, 0x04, 0xEC, 0x11, 0x64, 0x8D, 0x1E, 0x00},
	  {0x00, 0x00, 0xFC, 0x00, 0x60, 0xD9, 0xB0, 0x00},
	  {0x00, 0x09, 0x7C, 0x03, 0x80, 0xD9, 0x60, 0x00},
	  {0x00, 0x00, 0x76, 0x3F, 0xFC, 0x3C, 0x00, 0x00},
	  {0x00, 0x00, 0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x00},
	  {0x00, 0x00, 0xF8, 0x1F, 0xCF, 0xFE, 0x00, 0x00},
	  {0x00, 0x07, 0x30, 0x0F, 0xC4, 0xFF, 0xC0, 0x00},
	  {0x00, 0x06, 0x20, 0x07, 0xC0, 0x13, 0xC0, 0x00},
	  {0x00, 0x06, 0x21, 0x07, 0xC0, 0x12, 0x40, 0x00},
	  {0x00, 0x06, 0x23, 0x87, 0xC4, 0x1A, 0x40, 0x00},
	  {0x00, 0x06, 0x23, 0x83, 0xC0, 0x12, 0x40, 0x00},
	  {0x00, 0x06, 0x03, 0x83, 0xC0, 0x1A, 0x40, 0x00},
	  {0x00, 0x06, 0x03, 0x83, 0xC0, 0x1A, 0x40, 0x00},
	  {0x00, 0x06, 0x03, 0x83, 0xC4, 0x1A, 0x40, 0x00},
	  {0x00, 0x06, 0x03, 0x83, 0xFF, 0xFE, 0x40, 0x00},
	  {0x00, 0x06, 0x03, 0x83, 0xFF, 0xFF, 0xC0, 0x00},
	  {0x00, 0x06, 0x23, 0x87, 0xC1, 0x89, 0x40, 0x00},
	  {0x00, 0x06, 0x23, 0x07, 0xC1, 0x89, 0x40, 0x00},
	  {0x00, 0x06, 0x20, 0x0F, 0xC0, 0x88, 0x40, 0x00},
	  {0x00, 0x06, 0x30, 0x0F, 0xC8, 0x8C, 0xC0, 0x00},
	  {0x00, 0x06, 0x30, 0x1F, 0xC8, 0x84, 0xC0, 0x00},
	  {0x00, 0x07, 0xFC, 0x7F, 0xC8, 0x84, 0xC0, 0x00},
	  {0x00, 0x06, 0x03, 0xFF, 0xC8, 0xA4, 0xC0, 0x00},
	  {0x00, 0x07, 0xF0, 0x07, 0xC8, 0x84, 0xC0, 0x00},
	  {0x00, 0x06, 0x96, 0xFF, 0xC8, 0x04, 0xC0, 0x00},
	  {0x00, 0x06, 0x92, 0x67, 0xC9, 0x04, 0xC0, 0x00},
	  {0x00, 0x06, 0x92, 0x4F, 0xC1, 0x24, 0xC0, 0x00},
	  {0x00, 0x07, 0x82, 0x67, 0xC1, 0x2F, 0xC0, 0x00},
	  {0x00, 0x01, 0xFE, 0x47, 0xC3, 0xFF, 0x00, 0x00},
	  {0x00, 0x00, 0x0F, 0xE7, 0xFF, 0xF0, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x12, 0x82, 0x80, 0x00, 0x00},
	  {0x00, 0x00, 0x01, 0x50, 0x8A, 0x80, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x12, 0x82, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x3C, 0x18, 0x38, 0x38, 0x38, 0xF8, 0x79, 0xF0},
	  {0x3F, 0x3C, 0x7C, 0x38, 0x38, 0xFC, 0xFD, 0xF8},
	  {0x33, 0x3C, 0xE0, 0x38, 0x3C, 0xCD, 0xCD, 0xCC},
	  {0x33, 0x34, 0x78, 0x38, 0x6C, 0xFD, 0x8F, 0xDC},
	  {0x33, 0xB6, 0x3C, 0x38, 0x6C, 0xFD, 0x8F, 0xF8},
	  {0x33, 0x7E, 0x0C, 0x38, 0x7E, 0xC7, 0xCD, 0xF8},
	  {0x3F, 0x7E, 0x6C, 0x3A, 0x7E, 0xEE, 0xDD, 0x98},
	  {0x3E, 0xE7, 0x7C, 0x3F, 0xC6, 0xFC, 0xF9, 0xDC},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x02, 0x7E, 0x5A, 0xA4, 0x56, 0xA0, 0x00},
	  {0x00, 0x02, 0xD0, 0x48, 0x34, 0x4A, 0xA0, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}};


static uint8_t logo_Labor10thAnniversairy_getChunk(unsigned char const nBitPlane,
                                                   unsigned char const nChunkX,
                                                   unsigned char const nChunkY,
                                                   unsigned int const nFrame)
{
	assert(nBitPlane < 2);
	assert(nChunkX < 8);
	assert(nChunkY < 89);

	return pgm_read_byte(&aLabor10thAnniversairyBitmap[nBitPlane][nChunkY][nChunkX]);
}


void logo_Labor10thAnniversairy()
{
	// width 64, height 89, 2 bitplanes (4 colors), 600 frames à 50ms (20 fps)
	bitmap_scroll(64, 89, 2, 600, 50, 1, 1, logo_Labor10thAnniversairy_getChunk);
}
