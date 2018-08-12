#include "../config.h"
#include <stdint.h>
#include "../random/prng.h"
#include "../pixel.h"
#include "../util.h"
#include "../compat/pgmspace.h"

#define LINE_DISTANCE 4
#define SINTAB_LENGTH 16
#define SIN_MAX 6

// lolshield etc.
#define SMALL (NUM_ROWS < 10)
#define PGM(x) pgm_read_byte(&(x))

uint8_t sintab_base[SINTAB_LENGTH] =
	{
			0, // 0
			14,
			18, // 1
			24,
			28,	// 2
			32,
			36, // 2
			42,
			44,	// 3
			46,
			50, // 3
			52,
			56, // 4
			60,
			62,	// 4
			64
	};

/**
 * Shifts the Pixmap one px right
 */
static void move(){
		unsigned char plane, row, byte;

		for(plane=0; plane<NUMPLANE; plane++){
			for(row=NUM_ROWS; row--;){
				for(byte=0; byte < LINEBYTES; byte++){
					pixmap[plane][row][byte] = pixmap[plane][row][byte] >> 1;
					if(byte < LINEBYTES-1){
						pixmap[plane][row][byte] =
								pixmap[plane][row][byte] |
								(pixmap[plane][row][byte+1] & 0x01) << 7;
					}
				}
			}
		}
}


void dna(){

	uint8_t height = NUM_ROWS * 3 / 4 ;
	// small screen: bigger helix
	if(SMALL){
		height = NUM_ROWS;
	}

	uint8_t sin_length = 16;

	uint8_t mid = NUM_ROWS / 2;
	uint8_t draw_line = 0;

	uint8_t top = 0;
	uint8_t bottom = 0;

	uint8_t top_color = 3;
	uint8_t bottom_color = 2;

	uint32_t c = 600;
	uint8_t sintab[sin_length];

	uint8_t sinpos = 0;

	int8_t direction = 1;
	// disable up- and down-movement on small screens
	if(SMALL){
		direction = 0;
	}


	clear_screen(0);

	/* calculate sinus wave */
	for(int i = 0; i < sin_length / 2; i++){
		int index = i * SINTAB_LENGTH * 2 / sin_length;
		sintab[i] = sintab_base[index] * height / 128;
		sintab[sin_length - i - 1] = sintab[i];

	}

	while(c--){
		top = mid - sintab[sinpos];
		bottom = mid + sintab[sinpos];

		setpixel((pixel){NUM_COLS-1,top}, top_color);
		setpixel((pixel){NUM_COLS-1,bottom}, bottom_color);

		if(draw_line == 0){
			for(uint8_t linex = top+1; linex < bottom; linex++){
				setpixel((pixel){NUM_COLS-1, linex}, 1);
			}
			setpixel((pixel){NUM_COLS-1, mid}, 1);
		}

		if(sinpos == sin_length-1){
			if(mid-SIN_MAX <= 0 && direction){
				direction = 1;
			}
			if(mid+SIN_MAX >= NUM_ROWS-1 && direction){
				direction = -1;
			}
			mid = mid + (random8() > 200) * direction;
		}

		draw_line = (draw_line+1) % LINE_DISTANCE;
		sinpos = (sinpos + 1) % sin_length;

		if (sinpos == 0){
			uint8_t tmp_color = top_color;
			top_color = bottom_color;
			bottom_color = tmp_color;
		}

		b2d_wait(50);
		move();
	}


}

