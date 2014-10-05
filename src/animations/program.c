
#include <stdbool.h>
#include "../config.h"
#include "../compat/pgmspace.h"
#include "../random/prng.h"
#include "../pixel.h"
#include "../util.h"

#define RANDOM8() (random8())


#ifdef ANIMATION_TESTS
void test_level(unsigned char level, bool debug){
	for (unsigned char y=NUM_ROWS;y--;){
		for (unsigned char x=NUM_COLS;x--;){
			setpixel((pixel){x,y}, level);
			wait(5);
		}
	}
	if (!debug) {
		wait(2000);
	}
}

void test_palette(bool debug){
	for (unsigned char y=NUM_ROWS;y--;){
		for (unsigned char x=NUM_COLS;x--;){
			setpixel((pixel){x,y}, y%4);
			// wait(1);
		}
	}
	if (!debug) {
		wait(2000);
	}
}

void test_palette2(bool debug){
	for (unsigned char x=NUM_COLS;x--;){
		for (unsigned char y=NUM_ROWS;y--;){
			setpixel((pixel){x,y}, x%4);
		}
	}
	if (!debug) {
		wait(1000);
		for (unsigned char x=NUM_COLS;x--;){
			// shift image right
			shift_pixmap_l();
			wait(30);
		}
	}
}
#endif


#ifdef ANIMATION_SPIRAL
void spiral(int delay) {
	clear_screen(0);
	static signed char const PROGMEM delta[5] = { 0, -1, 0, 1, 0 };
	unsigned char length[2] = { NUM_ROWS, NUM_COLS - 1 };
	unsigned char x = NUM_COLS - 1, y = NUM_ROWS, i = 0;

	while (length[i & 0x01]) {
		for (unsigned char j = 0; j < length[i & 0x01]; ++j) {
			x += pgm_read_byte(&delta[i]);
			y += pgm_read_byte(&delta[i + 1]);
			setpixel((pixel){x, y}, NUMPLANE);
			wait(delay);
		}
		length[i++ & 0x01]--;
		i %= 4;
	}
	i = (i + 2u) % 4u;
	while (length[0] <= NUM_ROWS && length[1] < NUM_COLS) {
		for (unsigned char j = 0; j < length[i & 0x01]; ++j) {
			setpixel((pixel){x, y}, 0);
			x += pgm_read_byte(&delta[i]);
			y += pgm_read_byte(&delta[i + 1]);
			wait(delay);
		}
		length[(i += 3) & 0x01]++;
		i %= 4;
	}
}
#endif


#ifdef ANIMATION_JOERN1
void joern1(){
unsigned char i, j, x;
	unsigned char rolr=0x01 , rol;
	clear_screen(3);
	for(i = 0; i< 80;i++){
		rol = rolr;
		for(j = 0 ;j < NUM_ROWS; j++){
			for(x=0;x<LINEBYTES;x++)
				pixmap[2][j][x] = rol;
			if((rol<<=1)==0)rol = 0x01;
		}
		if((rolr<<=1) == 0) rolr = 1;
		wait(100);
	}
}
#endif


#ifdef ANIMATION_CHECKERBOARD
void checkerboard(unsigned char times){
	while (times--) {
		for (unsigned char row = 0; row < NUM_ROWS; ++row) {
			for (unsigned char col = 0; col < NUM_COLS; ++col) {
				setpixel((pixel){col, row}, (times ^ row ^ col) & 0x01 ? 0 : 3);
			}
		}
		wait(200);
	}
}
#endif


#ifdef ANIMATION_FIRE
#ifndef FIRE_CYCLES
	#define FIRE_CYCLES 800
#endif
#define FIRE_Y (NUM_ROWS + 3)
void fire()
{
	unsigned char y, x;
	unsigned int  t;
	unsigned char world[NUM_COLS][FIRE_Y];   // double buffer

	for (t = 0; t < FIRE_CYCLES; t++) {
		// diffuse
		for (y = 1; y < FIRE_Y; y++) {
			for (x = 1; x < NUM_COLS - 1; x++) {
				world[x][y - 1] =
						(FIRE_N * world[x - 1][y] +
						 FIRE_S * world[    x][y] +
						 FIRE_N * world[x + 1][y]) / FIRE_DIV;
			};

			world[0][y - 1] =
					(FIRE_N * world[NUM_COLS - 1][y] +
					 FIRE_S * world[           0][y] +
					 FIRE_N * world[           1][y]) / FIRE_DIV;

			world[NUM_COLS - 1][y - 1] =
					(FIRE_N * world[0           ][y] +
					 FIRE_S * world[NUM_COLS - 1][y] +
					 FIRE_N * world[NUM_COLS - 2][y]) / FIRE_DIV;
		}

		// update lowest line
		for (x = 0; x < NUM_COLS; x++) {
			world[x][FIRE_Y - 1] = RANDOM8();
		}

		// copy to screen
		for (y = 0; y < NUM_ROWS; y++) {
			for (x = 0; x < NUM_COLS; x++) {
				setpixel((pixel) {x,y}, world[x][y] >> 5);
			}
		}

		wait(FIRE_DELAY);
	}
}
#endif


#ifdef ANIMATION_RANDOM_BRIGHT
/**
 * void random_bright(void)
 *  by Daniel Otte
 */
void random_bright(unsigned int cycles) {
	while (cycles--) {
		for (unsigned char p = NUMPLANE; p--;) {
			for (unsigned char y = NUM_ROWS; y--;) {
				for (unsigned char x = LINEBYTES; x--;) {
					if (p < (NUMPLANE - 1)) {
						pixmap[p][y][x] |= pixmap[p + 1][y][x];
					}
					pixmap[p][y][x] = random8();
				}
			}
		}
		wait(200);
	}
}
#endif
