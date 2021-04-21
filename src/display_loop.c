
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "scrolltext/scrolltext.h"
#include "animations/snake.h"
#include "animations/program.h"
#include "animations/matrix.h"
#include "animations/dna.h"
#include "animations/gameoflife.h"
#include "animations/stonefly.h"
#include "animations/flyingdots.h"
#include "animations/breakout_demo.h"
#include "animations/ltn_ant.h"
#include "animations/bitmapscroller/laborlogo.h"
#include "animations/bitmapscroller/amphibian.h"
#include "animations/bitmapscroller/outofspec.h"
#include "animations/bitmapscroller/fairydust.h"
#include "animations/bitmapscroller/thisisnotdetroit.h"
#include "animations/fpmath_patterns.h"
#include "animations/mherweg.h"
#include "animations/moire.h"
#include "animations/blackhole.h"
#include "animations/squares.h"
#include "util.h"
#ifdef ANIMATION_TIME
#include "animations/borg_time.h"
#endif
#include "borg_hw/borg_hw.h"
#include "can/borg_can.h"
#include "random/prng.h"
#include "random/persistentCounter.h"
#include "mcuf/mcuf.h"
#include "menu/menu.h"
#include "pixel.h"
#ifdef SMALLANIMATION_ROWWALK
#  include "smallani/rowwalk.h"
#endif
#ifdef SMALLANIMATION_COLWALK
#  include "smallani/colwalk.h"
#endif
#ifdef SMALLANIMATION_ROWBOUNCE
#  include "smallani/rowbounce.h"
#endif
#ifdef SMALLANIMATION_COLBOUNCE
#  include "smallani/colbounce.h"
#endif

#ifdef JOYSTICK_SUPPORT
#  include "joystick/joystick.h"
#endif

#include "user/user_loop.h"

volatile unsigned char oldMode, oldOldmode, reverseMode, mode;

jmp_buf newmode_jmpbuf;

#ifndef MENU_SUPPORT
void snake_game(void);
void tetris(void);
void tetris_bastet(void);
void tetris_fp(void);
void borg_invaders(void);
void borg_breakout(unsigned char demomode);
#endif

void display_loop(){
//	mcuf_serial_mode();
#ifdef RANDOM_SUPPORT
	srandom32(percnt_get(&g_reset_counter, &g_reset_counter_idx));
	percnt_inc(&g_reset_counter, &g_reset_counter_idx);
#endif
	mode = setjmp(newmode_jmpbuf);

#ifdef JOYSTICK_SUPPORT
	// in case we get here via mode jump, we (re)enable joystick queries
	waitForFire = 1;
#endif

	oldOldmode = oldMode;

#ifdef JOYSTICK_SUPPORT
	waitForFire = 1;
#endif

	for(;;){
#ifndef MENU_SUPPORT
		clear_screen(0);
#endif
		oldMode = mode;

		switch(mode++) {

#ifdef ANIMATION_SCROLLTEXT
		case 1:
			scrolltext(scrolltext_text);

	#ifdef RANDOM_SUPPORT
			{
				char a[28];
				sprintf(a,"</# counter == %lu  ",
					(unsigned long)percnt_get(&g_reset_counter, &g_reset_counter_idx));
				scrolltext(a);
			}
	#endif
#endif
#ifdef ANIMATION_TIME
#ifndef ANIMATION_SCROLLTEXT
		case 1:
#endif
			time_anim();
			break;
#else
#ifdef ANIMATION_SCROLLTEXT
			break;
#endif
#endif

#ifdef ANIMATION_SPIRAL
#		ifndef SPIRAL_DELAY
#			define SPIRAL_DELAY 5
#		endif

		case 2:
			spiral(SPIRAL_DELAY);
			break;
#endif

#ifdef ANIMATION_JOERN1
		case 3:
			joern1();
			break;
#endif

#ifdef ANIMATION_SNAKE
		case 4:
			snake_animation();
			break;
#endif

#ifdef ANIMATION_CHECKERBOARD
		case 5:
			checkerboard(20);
			break;
#endif

#ifdef ANIMATION_FIRE
		case 6:
			fire();
			break;
#endif

#ifdef ANIMATION_TIME
		case 7:
			time_anim();
			break;
#endif

#ifdef ANIMATION_MATRIX
		case 8:
			matrix();
			break;
#endif

#ifdef ANIMATION_RANDOM_BRIGHT
		case 9:
			random_bright(30);
			break;
#endif

#ifdef ANIMATION_STONEFLY
		case 10:
			stonefly();
			break;
#endif

#ifdef ANIMATION_GAMEOFLIFE
		case 11:
			gameoflife();
			break;
#endif

#ifdef ANIMATION_FLYINGDOTS
		case 12:
			flyingdots();
			break;
#endif

#ifdef ANIMATION_BREAKOUT
		case 13:
			breakout_demo();
			break;
#endif

#ifdef ANIMATION_MHERWEG
		case 14:
			mherweg();
			break;
#endif

#ifdef ANIMATION_MOIRE
		case 15:
			moire();
			break;
#endif

#ifdef ANIMATION_TIME
		case 16:
			time_anim();
			break;
#endif

#ifdef ANIMATION_LTN_ANT
		case 17:
			ltn_ant();
			break;
#endif

#ifdef ANIMATION_LABORLOGO
		case 18:
			laborlogo();
			break;
#endif

#ifdef ANIMATION_AMPHIBIAN
		case 19:
			amphibian();
			break;
#endif

#ifdef ANIMATION_LOGO_OOS
		case 20:
			logo_OutOfSpec();
			break;
#endif

#ifdef ANIMATION_FAIRYDUST
		case 21:
			fairydust();
			break;
#endif

#ifdef ANIMATION_PLASMA
		case 22:
			plasma();
			break;
#endif

#ifdef ANIMATION_PSYCHEDELIC
		case 23:
			psychedelic();
			break;
#endif

#ifdef ANIMATION_BLACKHOLE
		case 24:
			blackhole();
			break;
#endif

#ifdef ANIMATION_SQUARES
		case 25:
			squares();
			break;
#endif

#ifdef ANIMATION_DNA
		case 26:
			dna();
			break;
#endif

#ifdef ANIMATION_SURFACE_WAVE
		case 27:
			surfaceWave();
			break;
#endif

#ifdef ANIMATION_THISISNOTDETROIT
		case 28:
			logo_thisIsNotDetroit();
			break;
#endif

#ifdef ANIMATION_TESTS
		case 31:
			test_level(1, false);
			break;

		case 32:
			test_level(2, false);
			break;

		case 33:
			test_level(3, false);
			break;

		case 35:
			test_palette(false);
			test_palette2(false);
			break;
#endif

#ifdef SMALLANIMATION_ROWWALK
		case 36:
		  rowwalk(SMALLANIMATION_ROWWALK_COUNT,SMALLANIMATION_ROWWALK_SPEED);
		  break;
#endif

#ifdef SMALLANIMATION_COLWALK
		case 37:
		  colwalk(SMALLANIMATION_COLWALK_COUNT,SMALLANIMATION_COLWALK_SPEED);
		  break;
#endif
#ifdef SMALLANIMATION_COLBOUNCE
		case 38:
		  colbounce(SMALLANIMATION_COLBOUNCE_COUNT,SMALLANIMATION_COLBOUNCE_SPEED);
		  break;
#endif
#ifdef SMALLANIMATION_ROWBOUNCE
		case 39:
		  rowbounce(SMALLANIMATION_ROWBOUNCE_COUNT,SMALLANIMATION_ROWBOUNCE_SPEED);
		  break;
#endif

#include "user/user_loop.c"

#ifdef MENU_SUPPORT
		case 0xFDu:
			mode = 1;
			break;

		case 0xFEu:
			menu();
			mode = oldOldmode;
			break;
#else

		case 0xFDu:
#ifdef JOYSTICK_SUPPORT
			if (JOYISFIRE)
				mode = 0xFEu;
			else
#endif
				mode = 1;
			break;

		case 0xFEu:
#ifdef JOYSTICK_SUPPORT
			waitForFire = 0;   // avoid circular jumps
			while (JOYISFIRE) {
                // b2d_wait until user released the fire button
                b2d_wait(25);
            }
#else
			b2d_wait(25);          // b2d_wait for button to settle
#endif

#  ifdef GAME_TETRIS
			tetris();
#  endif

#  ifdef GAME_BASTET
			tetris_bastet();
#  endif

#  ifdef GAME_TETRIS_FP
			tetris_fp();
#  endif

#  ifdef GAME_SPACE_INVADERS
			borg_invaders();
#  endif

#  ifdef GAME_SNAKE
			snake_game();
#  endif

#  ifdef GAME_BREAKOUT
			borg_breakout(0);
#  endif

#ifdef JOYSTICK_SUPPORT
			while (JOYISFIRE); // avoid an unwanted restart of the game loop
#endif
			b2d_wait(25);          // b2d_wait for button to settle
			mode = oldOldmode; // restore old animation mode
#ifdef JOYSTICK_SUPPORT
			waitForFire = 1;   // reenable joystick query of the b2d_wait() function
#endif
			break;
#endif

#ifdef ANIMATION_OFF
		case 0xFFu:
			off();
			break;
#endif
		default:
            b2d_wait(5);
			if (reverseMode) {
				if (reverseMode-- == (mode - 1)) {
					mode -= 2;
				} else {
					reverseMode = 0;
				}
			}
			break;
		}
	}
}
