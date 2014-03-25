/*
 * kart.c
 *
 *  Created on: 07.02.2014
 *      Author: Stefan Kinzel
 */


#include <assert.h>
#include <stdint.h>
#include "../../config.h"
#include "../../compat/pgmspace.h"
#include "../../pixel.h"
#include "../../random/prng.h"
#include "../../util.h"
#include "../../joystick/joystick.h"
#include "../../menu/menu.h"
#include "../../scrolltext/scrolltext.h"
#include "kart.h"


#include <stdio.h>


#if defined MENU_SUPPORT && defined GAME_KART
// icon (TODO: convert to hex)
static const uint8_t icon[8] PROGMEM =
	{	0b11100001,
		0b11100001,
		0b11000011,
		0b10000111,
		0b10000111,
		0b11000011,
		0b11000011,
		0b10010111};

game_descriptor_t kart_game_descriptor __attribute__((section(".game_descriptors"))) =
{
	&kart_game,
	icon,
};
#endif

#define WAIT 20

#define DRIVEDIV 5
#define MOVEDIV 1
#define DECREASE_WIDTH_DIV 600
#define DIRECTION_DIV 20

#define KEY_IGNORE 10

#define CARCOLOR 3
#define BORDERCOLOR 2
#define LINECOLOR 1

#define CURVE_PROP 50

uint8_t borders[NUM_ROWS][2];
uint8_t key_ignore[2];

void kart_game(){

	// Initialisation
	uint8_t carpos = NUM_COLS / 2;
	uint32_t cycle = 0;
	uint8_t draw_line = 1;
	uint8_t width = NUM_COLS - 2;
	uint8_t middle = NUM_COLS / 2;
	char game_over[100] = "";

	key_ignore[0] = 0;
	key_ignore[1] = 0;

	clear_screen(0);

	// init border memory
	for(uint8_t row = 0; row < NUM_ROWS; row++){
		borders[row][0] = middle;
		borders[row][1] = NUM_COLS;
	}

	setpixel((pixel){carpos, NUM_ROWS-1}, CARCOLOR);

	// main loop
	while(1){

		// DECREASE WIDTH
		if(cycle % DECREASE_WIDTH_DIV == 0){
			width--;
		}

		// MOVE
		if (JOYISLEFT && key_ignore[0] <= 0){
			setpixel((pixel){carpos, NUM_ROWS-1}, 0);
			carpos++;

			key_ignore[0] = KEY_IGNORE;
			key_ignore[1] = 0;
		}else if (JOYISRIGHT && key_ignore[1] <= 0){
			setpixel((pixel){carpos, NUM_ROWS-1}, 0);
			carpos--;

			key_ignore[1] = KEY_IGNORE;
			key_ignore[0] = 0;
		}else if(!(JOYISRIGHT || JOYISLEFT)){
			key_ignore[1] = 0;
			key_ignore[0] = 0;
		}

		if(check_collision(carpos)){
			break;
		}


		// DIRECTION-STEP
		if(cycle % DIRECTION_DIV == 0){
			// generate a route
			int rnd = random8();
			if(rnd < CURVE_PROP && middle-(width/2) > 1){
				middle--;
			}else if(rnd > 256-CURVE_PROP && middle+(width/2) < NUM_COLS-1){
				middle++;
			}
		}

		// DRIVE-STEP
		if(cycle % DRIVEDIV == 0){
			// shift pixmap down
			drive();

			// save border state
			save_borders(middle, width);

			// draw new first line
			unsigned int px;
			for(px=0; px < NUM_COLS; px++){
				if(px<middle-(width/2) || px >= middle+(width/2)){
					setpixel((pixel){px, 0}, BORDERCOLOR);
				}
			}
		}

		// toggle drawing the middle line
		if(cycle % (DRIVEDIV*4) == 0){
			draw_line = 1-draw_line;
		}

		// paint middle line
		if(width > 6 && draw_line){
			setpixel((pixel){middle, 0}, LINECOLOR);
		}

		// Paint car
		setpixel((pixel){carpos, NUM_ROWS-1}, CARCOLOR);

		cycle++;

		if(key_ignore[0] > 0){
			key_ignore[0]--;
		}
		if(key_ignore[1] > 0){
			key_ignore[1]--;
		}
		wait(WAIT);
	}

	snprintf(game_over, sizeof(game_over), "</#Game Over, Score: %i",  cycle);
	scrolltext(game_over);
}

/**
 * Shifts the Pixmap one px down
 */
void drive(void){

	unsigned char plane, row, byte;

	for(plane=0; plane<NUMPLANE; plane++){
		for(row=NUM_ROWS-1;row>0; row--){
			for(byte=0; byte < LINEBYTES; byte++){
				pixmap[plane][row][byte] = pixmap[plane][row-1][byte];
			}
		}
		for(byte=0; byte < LINEBYTES; byte++){
			pixmap[plane][row][byte] = 0x00;
		}
	}

}

/**
 * Save the border state at the top line, so collision detection can
 * work in the last line (where the car is).
 */
void save_borders(uint8_t middle, uint8_t width){
	uint8_t row;
	for(row = NUM_ROWS-1; row > 0 ; row--){
		borders[row][0] = borders[row-1][0];
		borders[row][1] = borders[row-1][1];
	}
	borders[0][0] = middle;
	borders[0][1] = width;
}

/**
 * check if collision occours
 */
uint8_t check_collision(uint8_t carpos){
	uint8_t middle = borders[NUM_ROWS-1][0];
	uint8_t width = borders[NUM_ROWS-1][1];

	return ( carpos<middle-(width/2) || carpos >= middle+(width/2) );
}
