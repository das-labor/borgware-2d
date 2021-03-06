#include <stdint.h>
#include <assert.h>

#include "../../compat/pgmspace.h"
#include "bitmapscroller.h"
#include "fairydust.h"

static uint8_t const nMargin = 5;          /** margin above and below bitmap */
static uint8_t const nRayStartOffset = 15; /** offset of the animated part */
static uint8_t const nRayStopOffset = 25;  /** last offset of that part */

/**
 * complete bitmap of the fairy dust with a full ray
 */
static uint8_t const aFairydustBm[2][30][9] PROGMEM =
	{{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x0e, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0xf0, 0x08, 0x80},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xe0, 0x07, 0xc0},
	  {0x00, 0x00, 0x00, 0x00, 0x01, 0x7e, 0x40, 0x00, 0xfe},
	  {0x00, 0x00, 0x00, 0x0f, 0x05, 0xfa, 0x00, 0x01, 0xf8},
	  {0x00, 0x00, 0x01, 0x80, 0xd7, 0xef, 0x38, 0x0b, 0xf0},
	  {0x00, 0x00, 0x08, 0x0f, 0xf9, 0xb3, 0xf8, 0x2f, 0xf0},
	  {0x00, 0x00, 0x14, 0xf7, 0xf9, 0xff, 0xfe, 0x3f, 0xf0},
	  {0x00, 0x00, 0x30, 0x7f, 0xfa, 0x7f, 0xff, 0xff, 0xc0},
	  {0x00, 0x01, 0xcf, 0xff, 0xfd, 0x7f, 0xff, 0xff, 0x80},
	  {0x00, 0x04, 0x7f, 0xff, 0xf8, 0xff, 0xff, 0xff, 0x40},
	  {0x00, 0x03, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xfe, 0x80},
	  {0x00, 0x05, 0xff, 0xbf, 0x87, 0xff, 0xff, 0xfd, 0x00},
	  {0x00, 0x02, 0xce, 0x62, 0x3f, 0xff, 0xff, 0xf8, 0x00},
	  {0x00, 0x00, 0x41, 0x7f, 0xff, 0xff, 0xff, 0xe8, 0x00},
	  {0x00, 0x00, 0x02, 0xe1, 0xff, 0xff, 0xff, 0xc0, 0x00},
	  {0x00, 0x00, 0x05, 0xa3, 0xff, 0xff, 0xfe, 0xc0, 0x00},
	  {0x00, 0x00, 0x1a, 0x00, 0xff, 0xff, 0xf2, 0x00, 0x00},
	  {0x00, 0x00, 0x67, 0x41, 0xff, 0xfe, 0x60, 0x00, 0x00},
	  {0x00, 0x01, 0x01, 0xf8, 0xe0, 0x1c, 0x00, 0x00, 0x00},
	  {0x00, 0x18, 0x1f, 0xeb, 0xff, 0xc0, 0x00, 0x00, 0x00},
	  {0x01, 0xc0, 0xe0, 0xf1, 0xff, 0xa0, 0x00, 0x00, 0x00},
	  {0x0e, 0x18, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00},
	  {0x33, 0x80, 0x00, 0x17, 0xff, 0x00, 0x00, 0x00, 0x00},
	  {0x70, 0x00, 0x00, 0x5f, 0xfa, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x01, 0x7f, 0xe8, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x01, 0xff, 0xa0, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x7c, 0x80, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00}},

	 {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x07, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x03, 0xfc},
	  {0x00, 0x00, 0x00, 0x7f, 0x0f, 0xf0, 0xf0, 0x07, 0xf8},
	  {0x00, 0x00, 0x07, 0xf0, 0x07, 0xcf, 0xfc, 0x1f, 0xf0},
	  {0x00, 0x00, 0x0f, 0x08, 0x00, 0xff, 0xff, 0xff, 0xe0},
	  {0x00, 0x00, 0x0f, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xe0},
	  {0x00, 0x00, 0x3f, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xc0},
	  {0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80},
	  {0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00},
	  {0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00},
	  {0x00, 0x01, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xfc, 0x00},
	  {0x00, 0x00, 0x00, 0xf3, 0xff, 0xff, 0xff, 0xf0, 0x00},
	  {0x00, 0x00, 0x01, 0xf3, 0xff, 0xff, 0xff, 0xe0, 0x00},
	  {0x00, 0x00, 0x02, 0x41, 0xff, 0xff, 0xff, 0x00, 0x00},
	  {0x00, 0x00, 0x04, 0x01, 0xff, 0xff, 0xfc, 0x00, 0x00},
	  {0x00, 0x00, 0x18, 0x80, 0xff, 0xff, 0x80, 0x00, 0x00},
	  {0x00, 0x00, 0xff, 0xc1, 0xff, 0xe0, 0x00, 0x00, 0x00},
	  {0x00, 0x07, 0xe0, 0xf1, 0xff, 0xe0, 0x00, 0x00, 0x00},
	  {0x00, 0x3f, 0x00, 0x03, 0xff, 0xc0, 0x00, 0x00, 0x00},
	  {0x01, 0xe0, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x00},
	  {0x0c, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00}}};


/**
 * Animated part of the bitmap (with different strengths of the ray)
 */
static uint8_t const aRay[2][44][4] PROGMEM =
	{{{0x00, 0x00, 0x41, 0x7f}, // frame 1
	  {0x00, 0x00, 0x02, 0xe1},
	  {0x00, 0x00, 0x05, 0xa3},
	  {0x00, 0x00, 0x1a, 0x00},
	  {0x00, 0x00, 0x67, 0x41},
	  {0x00, 0x01, 0x01, 0xf8},
	  {0x00, 0x18, 0x1f, 0xeb},
	  {0x00, 0x20, 0xe0, 0xf1},
	  {0x00, 0x18, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x17},
	  {0x00, 0x00, 0x00, 0x5f},
	  {0x00, 0x00, 0x41, 0x7f}, // frame 2
	  {0x00, 0x00, 0x02, 0xe1},
	  {0x00, 0x00, 0x05, 0xa3},
	  {0x00, 0x00, 0x1a, 0x00},
	  {0x00, 0x00, 0x27, 0x41},
	  {0x00, 0x00, 0x19, 0xf8},
	  {0x00, 0x00, 0x07, 0xeb},
	  {0x00, 0x00, 0x00, 0xf1},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x17},
	  {0x00, 0x00, 0x00, 0x5f},
	  {0x00, 0x00, 0x40, 0x8f}, // frame 3
	  {0x00, 0x00, 0x01, 0x11},
	  {0x00, 0x00, 0x02, 0x43},
	  {0x00, 0x00, 0x04, 0x00},
	  {0x00, 0x00, 0x08, 0x81},
	  {0x00, 0x00, 0x06, 0x00},
	  {0x00, 0x00, 0x00, 0x13},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x17},
	  {0x00, 0x00, 0x00, 0x5f},
	  {0x00, 0x00, 0x40, 0x0f}, // frame 4
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x17},
	  {0x00, 0x00, 0x00, 0x5f}},

	 {{0x00, 0x00, 0x00, 0xf3}, // frame 1
	  {0x00, 0x00, 0x01, 0xf3},
	  {0x00, 0x00, 0x02, 0x41},
	  {0x00, 0x00, 0x04, 0x01},
	  {0x00, 0x00, 0x18, 0x80},
	  {0x00, 0x00, 0xff, 0xc1},
	  {0x00, 0x07, 0xe0, 0xf1},
	  {0x00, 0x1f, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x07},
	  {0x00, 0x00, 0x00, 0x0f},
	  {0x00, 0x00, 0x00, 0x3f},
	  {0x00, 0x00, 0x00, 0xf3}, // frame 2
	  {0x00, 0x00, 0x01, 0xf3},
	  {0x00, 0x00, 0x02, 0x41},
	  {0x00, 0x00, 0x04, 0x01},
	  {0x00, 0x00, 0x18, 0x80},
	  {0x00, 0x00, 0x07, 0xc1},
	  {0x00, 0x00, 0x00, 0xf1},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x07},
	  {0x00, 0x00, 0x00, 0x0f},
	  {0x00, 0x00, 0x00, 0x3f},
	  {0x00, 0x00, 0x00, 0x73}, // frame 3
	  {0x00, 0x00, 0x00, 0xe3},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x01, 0xc1},
	  {0x00, 0x00, 0x00, 0xe1},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x07},
	  {0x00, 0x00, 0x00, 0x0f},
	  {0x00, 0x00, 0x00, 0x3f},
	  {0x00, 0x00, 0x00, 0x03}, // frame 4
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x00},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x01},
	  {0x00, 0x00, 0x00, 0x03},
	  {0x00, 0x00, 0x00, 0x07},
	  {0x00, 0x00, 0x00, 0x0f},
	  {0x00, 0x00, 0x00, 0x3f}}};


static uint8_t fairydust_getChunk(unsigned char const nBitPlane,
                                  unsigned char const nChunkX,
                                  unsigned char const nChunkY,
                                  unsigned int const nFrame)
{
	assert(nBitPlane < 2);
	assert(nChunkX < 9);
	assert(nChunkY < (nMargin + 30 + nMargin));

	static uint8_t const nOffsetTable[] PROGMEM =
			{UINT8_MAX, 0, 11, 22, 33, 22, 11, 0};

	uint8_t const nOffset = pgm_read_byte(&nOffsetTable[(nFrame >> 1) % 8]);

	if (nChunkY < nMargin || nChunkY >= (nMargin + 30))
	{
		return 0;
	}
	else if ((nChunkX <= 3) &&
			(nChunkY >= (nRayStartOffset + nMargin)) &&
			(nChunkY <= (nRayStopOffset + nMargin)) &&
			(nOffset != UINT8_MAX))
	{
		uint8_t const row = nChunkY - nRayStartOffset - nMargin + nOffset;
		return pgm_read_byte(&aRay[nBitPlane][row][nChunkX]);
	}

	return pgm_read_byte(&aFairydustBm[nBitPlane][nChunkY - nMargin][nChunkX]);
}


void fairydust()
{
	bitmap_scroll(72, nMargin + 30 + nMargin, 2, 2400, 12, 1, 4,
			fairydust_getChunk);
}
