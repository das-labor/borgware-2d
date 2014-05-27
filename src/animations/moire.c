/**
 * \defgroup moire A moire like pattern.
 * @{
 */

/**
 * @file moire.c
 * @brief Implementation of a simple moire like pattern.
 * @author Christian Kroll
 */

#include "../config.h"
#include "../pixel.h"
#include "../util.h"

/**
 * Number of pixels of the complete border.
 */
#define NUMBER_OF_BORDER_PIXELS (2u * UNUM_COLS + 2u * (UNUM_ROWS - 2u))

/**
 * Draws a moire like pattern. Works best if the number of border pixels is a
 * multiple of the size of the gradient color map.
 */
void moire(void)
{
	// add rotating color map
#if NUMPLANE == 3 && NUM_COLS == 16 && NUM_ROWS == 16
	static unsigned char const gradient[] = {0, 1, 2, 3, 2, 1};
#	define WRAP 6u
#elif NUMPLANE == 3 && NUM_COLS == 14 && NUM_ROWS == 9
	static unsigned char const gradient[] = {0, 1, 1, 2, 2, 3, 3, 2, 2, 1, 1};
#	define WRAP 11u
#else
	static unsigned char gradient[NUMPLANE * 2u] = {0};
	for (unsigned char i = 1; i <= NUMPLANE; ++i)
	{
		gradient[i] = i;
		gradient[(NUMPLANE * 2) - i] = i;
	}
#	define WRAP (2u * NUMPLANE)
#endif

	unsigned int cycles = 30000;
	unsigned char pos = 0, color_index = 0;
	pixel p1 = (pixel){0 ,0};

	while(cycles--)
	{
		// walk around the border; do that by mapping a linear increasing value
		// to appropriate screen coordinates

		// pixel is between top right and top left corner
		if (pos < (NUM_COLS - 1))
		{
			p1.x = pos + 1;
		}
		// pixel is between top left and bottom left corner
		else if (pos < (NUM_COLS + NUM_ROWS - 2))
		{
			p1.y = pos - (NUM_COLS - 2);
		}
		// pixel is between bottom left and bottom right corner
		else if (pos < (2 * NUM_COLS + NUM_ROWS - 3))
		{
			p1.x = 2 * NUM_COLS + NUM_ROWS - 4 - pos;
		}
		// pixel is between bottom right and top left corner
		else
		{
			p1.y = 2 * NUM_COLS + 2 * NUM_ROWS - 5 - pos;
		}

		// second pixel in opposite direction
		pixel const p2 = (pixel){NUM_COLS - 1 - p1.x, NUM_ROWS - 1 - p1.y};

		// draw line right accross the display and switch to next color
		line(p1, p2, gradient[color_index++]);

		// if we have reached the origin, reset position, rotate color index and
		// wait for 40 ms (25 fps) to make the frame visible for human viewers
		if (++pos == NUMBER_OF_BORDER_PIXELS)
		{
			pos = 0;
			++color_index;
			wait(40);
		}
		// ensure the color index keeps within bounds
		color_index %= WRAP;
	}
}

/*@}*/
