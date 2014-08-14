#include "../config.h"
#include <stdint.h>
#include "../random/prng.h"
#include "../pixel.h"
#include "../util.h"


#define HEIGHT 12
#define LINE_DISTANCE 4
#define SIN_LENGTH 18
#define SIN_MAX 6

// uint8_t sin[SIN_LENGTH] = {0, 1, 2, 2, 3, 3, 4, 4, 4, 3, 3, 3, 2, 2, 1, 0};

uint8_t sintab[SIN_LENGTH] = {
		0,
		1,
		2,
		3,
		4,
		5,
		5,
		6,
		6,
		6,
		6,
		6,
		5,
		5,
		4,
		3,
		2,
		1,
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
	uint8_t mid = NUM_COLS / 2;
	uint8_t draw_line = 0;

	uint8_t top = 0;
	uint8_t bottom = 0;

	uint8_t top_color = 3;
	uint8_t bottom_color = 2;

	uint32_t c = 600;

	uint8_t sinpos = 0;

	int8_t direction = 1;

	clear_screen(0);

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

		if(sinpos == 0){
			if(mid-SIN_MAX <= 0){
				direction = 1;
			}
			if(mid+SIN_MAX >= NUM_ROWS-1){
				direction = -1;
			}
			mid = mid + (random8() > 200) * direction;
		}

		draw_line = (draw_line+1) % LINE_DISTANCE;
		sinpos = (sinpos + 1) % SIN_LENGTH;

		if(sinpos == 0){
			uint8_t tmp_color = top_color;
			top_color = bottom_color;
			bottom_color = tmp_color;
		}

		wait(50);
		move();
	}


}

