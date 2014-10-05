#ifndef PROGRAMM_H_
#define PROGRAMM_H_

#include <stdbool.h>
#include "../pixel.h"
#include "../util.h"


void test_level(unsigned char level, bool debug);
void test_palette(bool debug);
void test_palette2(bool debug);

#ifdef ANIMATION_OFF
inline static void off()
{
	clear_screen(0);

	while(1)
		wait(100);

}
#endif

void spiral(int delay);
void joern1();
void checkerboard(unsigned char times);
void fire();
void random_bright(unsigned cycles);

#endif /* PROGRAMM_H_ */
