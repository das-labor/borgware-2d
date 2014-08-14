#include "../config.h"
#include <stdint.h>
#include "../random/prng.h"
#include "../pixel.h"
#include "../util.h"


#define HEIGHT 12
#define LINE_DISTANCE 4
#define SIN_LENGTH 16

// uint8_t sin[SIN_LENGTH] = {0, 1, 2, 2, 3, 3, 4, 4, 4, 3, 3, 3, 2, 2, 1, 0};

uint8_t sintab[SIN_LENGTH] = {
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
		};

/*
uint8_t sintab[SIN_LENGTH] = {
1,
1,
2,
2,
3,
3,
4,
4,
5,
5,
5,
5,
6,
6,
6,
6,
6,
6,
6,
6,
6,
5,
5,
5,
5,
4,
4,
3,
3,
2,
2,
1,
};
*/

void dna(){
	uint8_t mid = NUM_COLS / 2;
	uint8_t draw_line = 0;

	uint8_t top = 0;
	uint8_t bottom = 0;

	uint8_t top_color = 3;
	uint8_t bottom_color = 2;

	uint32_t c = 10000;

	uint8_t sinpos = 0;

	uint8_t direction = 1;

	while(c--){
		top = mid - sintab[sinpos];
		bottom = mid + sintab[sinpos];

		setpixel((pixel){15,top}, top_color);
		setpixel((pixel){15,bottom}, bottom_color);

		if(draw_line == 0){
			for(uint8_t linex = top+1; linex < bottom; linex++){
				setpixel((pixel){15, linex}, 1);
			}
			setpixel((pixel){15, mid}, 1);
		}

		if(draw_line == 0){
			if(top <= 1){
				direction = 1;
			}
			if(bottom >= NUM_ROWS-1){
				direction = -1;
			}
			mid = mid + (random8() > 240) * direction;
		}

		draw_line = (draw_line+1) % LINE_DISTANCE;
		sinpos = (sinpos + 1) % SIN_LENGTH;

		if(sinpos == 0){
			uint8_t tmp_color = top_color;
			top_color = bottom_color;
			bottom_color = tmp_color;
		}

		move();

		wait(40);
	}


}

/**
 * Shifts the Pixmap one px right
 */
void move(){
		unsigned char plane, row, byte;

		for(plane=0; plane<=NUMPLANE; plane++){
			for(row=NUM_COLS;row>0; row--){
				for(byte=0; byte < LINEBYTES; byte++){
					pixmap[plane][row][byte] = pixmap[plane][row][byte] >> 1;
					if(byte < LINEBYTES-1){
						pixmap[plane][row][byte] =
								pixmap[plane][row][byte] |
								(pixmap[plane][row][byte+1] & 0b00000001) << 7;
					}
				}
			}
		}
}
