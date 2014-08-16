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
	{	0xE1, 	// 0b11100001,
		0xE1,	// 0b11100001,
		0xC3,	// 0b11000011,
		0x87,	// 0b10000111,
		0x87,	// 0b10000111,
		0xC3,	// 0b11000011,
		0xC3,	// 0b11000011,
		0x97,	// 0b10010111
	};

game_descriptor_t kart_game_descriptor __attribute__((section(".game_descriptors"))) =
{
	&kart_game,
	icon,
};
#endif

#define WAIT 15

#define DRIVE_DIV 10
#define DIRECTION_DIV 20
#define DECREASE_WIDTH_DIV 300
#define ACCELERATE_DIV 800

#define CURVE_PROP 50
#define OBSTACLE_PROP 30
// !! OBSTACLE_DIV in relation to drive steps
#define OBSTACLE_DIV 3
#define BOOST_CYCLES 150
#define BOOST_MULTIPLIER 5

#define KEY_IGNORE_INITIAL 15
#define KEY_IGNORE 7

#define CARCOLOR 3
#define BORDER_LIGHT 1
#define BORDER_DARK 2
#define OBSTACLE_COLOR 3

// borders = (middle, width, obstacle_pos)
uint8_t borders[NUM_ROWS][3];
uint32_t extra_score = 0;

void kart_game(){

	// Initialisation
	uint32_t decrease_width_div = DECREASE_WIDTH_DIV;
	uint8_t key_ignore[2];
	uint8_t drive_div = DRIVE_DIV;
	uint8_t carpos = NUM_COLS / 2;
	uint32_t cycle = 0;
	uint8_t light_border = 1;
	uint8_t width = NUM_COLS - 2;
	uint8_t middle = NUM_COLS / 2;
	// obstacle_pos == 0 --> no obstacle
	uint8_t obstacle_pos = 0;
	char game_over[100] = "";
	uint32_t boost_till = 0;
	uint8_t boost_multiplier = 1;

	key_ignore[0] = 0;
	key_ignore[1] = 0;

	clear_screen(0);

	// init street memory
	for(uint8_t row = 0; row < NUM_ROWS; row++){
		borders[row][0] = middle;
		borders[row][1] = NUM_COLS;
		borders[row][2] = 0;
	}

	// init street border
	line((pixel){0,0}, (pixel){0,NUM_ROWS}, BORDER_DARK);
	line((pixel){NUM_COLS-1,0}, (pixel){NUM_COLS-1,NUM_ROWS}, BORDER_DARK);

	setpixel((pixel){carpos, NUM_ROWS-1}, CARCOLOR);

	// main loop
	while(1){

		// DECREASE WIDTH
		if(cycle % decrease_width_div == 0){
			width--;
			decrease_width_div = decrease_width_div*2;
		}

		// INCREASE SPEED
		if(cycle % ACCELERATE_DIV == 0 ){
			drive_div--;
		}

		// MOVE
		if (JOYISLEFT && key_ignore[0] == 1){
			setpixel((pixel){carpos, NUM_ROWS-1}, 0);
			carpos++;

			key_ignore[0] = KEY_IGNORE;
			key_ignore[1] = 0;
		}else if (JOYISRIGHT && key_ignore[1] == 1){
			setpixel((pixel){carpos, NUM_ROWS-1}, 0);
			carpos--;

			key_ignore[1] = KEY_IGNORE;
			key_ignore[0] = 0;
		}else if (JOYISLEFT && key_ignore[0] <= 0){
			setpixel((pixel){carpos, NUM_ROWS-1}, 0);
			carpos++;

			key_ignore[0] = KEY_IGNORE_INITIAL;
			key_ignore[1] = 0;
		}else if (JOYISRIGHT && key_ignore[1] <= 0){
			setpixel((pixel){carpos, NUM_ROWS-1}, 0);
			carpos--;

			key_ignore[1] = KEY_IGNORE_INITIAL;
			key_ignore[0] = 0;
		}else if(JOYISUP && boost_till == 0){
			boost_till = cycle + BOOST_CYCLES;
			boost_multiplier = BOOST_MULTIPLIER;
		}else if(!(JOYISRIGHT || JOYISLEFT)){
			key_ignore[1] = 0;
			key_ignore[0] = 0;
		}

		if(check_collision(carpos)){
			break;
		}

		if(boost_till <= cycle && boost_multiplier > 1){
			boost_till = cycle + BOOST_CYCLES;
			boost_multiplier--;
		}else if(boost_multiplier == 1){
			boost_till = 0;
			extra_score += BOOST_CYCLES * BOOST_MULTIPLIER;
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
		if(cycle % (drive_div / boost_multiplier) == 0){
			// shift pixmap down
			drive();

			// save border state
			save_street(middle, width, obstacle_pos);

			// draw new first line
			unsigned int px;
			for(px=0; px < NUM_COLS; px++){
				if(px<middle-(width/2) || px >= middle+(width/2)){
					if(light_border){
						setpixel((pixel){px, 0}, BORDER_LIGHT);
					}else{
						setpixel((pixel){px, 0}, BORDER_DARK);
					}
				}
			}

			// toggle border color
			if(cycle % ((drive_div / boost_multiplier)*4) == 0){
				light_border = 1-light_border;
			}

			// set obstacle
			obstacle_pos = 0;
			if(cycle % OBSTACLE_DIV == 0 ){
				int rnd = random8();
				if(rnd < OBSTACLE_PROP){
					obstacle_pos = (random8() % width) + (middle - width/2);
					setpixel((pixel){obstacle_pos, 0}, OBSTACLE_COLOR);
				}
			}
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

	snprintf(game_over, sizeof(game_over), "</#Game Over, Score: %lu",  cycle);
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
 * Save the street state at the top line, so collision detection can
 * work in the last line (where the car is).
 */
void save_street(uint8_t middle, uint8_t width, uint8_t obstacle_pos){
	uint8_t row;
	for(row = NUM_ROWS-1; row > 0 ; row--){
		borders[row][0] = borders[row-1][0];
		borders[row][1] = borders[row-1][1];
		borders[row][2] = borders[row-1][2];
	}
	borders[0][0] = middle;
	borders[0][1] = width;
	borders[0][2] = obstacle_pos;

}

/**
 * check if collision occours
 */
uint8_t check_collision(uint8_t carpos){
	uint8_t middle = borders[NUM_ROWS-1][0];
	uint8_t width = borders[NUM_ROWS-1][1];
	uint8_t obstacle_pos = borders[NUM_ROWS-1][2];

	return ( carpos<middle-(width/2) || carpos >= middle+(width/2) || (obstacle_pos != 0 && carpos == obstacle_pos) );
}
