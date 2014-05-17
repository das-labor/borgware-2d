/* Copyright (c) 2010 Jan Lieven
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <limits.h>
#include "../config.h"
#include "../pixel.h"
#include "../util.h"
#include "../random/prng.h"

#if (UNUMCOLS <= (UCHAR_MAX / 2)) && (UNUMROWS <= (UCHAR_MAX / 2))
	typedef unsigned char coord_t;
#else
	typedef unsigned int coord_t;
#endif

#define P (1u)
#define NX (UNUM_COLS - 1u)
#define NY (UNUM_ROWS - 1u)

static coord_t const xdcomp[] = {0, NX,  0, P};
static coord_t const ydcomp[] = {P,  0, NY, 0};

typedef struct ant_s {
	coord_t  x,  y; /* current postion */
	coord_t ox, oy; /* previous position, used to dim out old pixels */
	unsigned char vector_index; /* index to one of (0,1),(1,0),(0,-1),(-1,0) */
} ant_t;

void ltn_ant() {
	clear_screen(0);

	ant_t ant;

	unsigned int cycles = 500;

	/* random start position and direction */
	ant.x = ant.ox = random8() % UNUM_COLS;
	ant.y = ant.oy = random8() % UNUM_ROWS;

	/* make sure we have a valid vector */
	ant.vector_index = random8() % 4u;

	while(cycles--) {
		/* if the pixel is turned off turn it on */
		if(get_pixel((pixel) {ant.x, ant.y}) == 0) {
			setpixel((pixel) {ant.x, ant.y}, NUMPLANE);
			ant.vector_index = (ant.vector_index + 3u) % 4u; // turn left

			/* dim the previous pixel */
			setpixel((pixel){ant.ox, ant.oy}, NUMPLANE - 1);

			/* memorize this position */
			ant.ox = ant.x;
			ant.oy = ant.y;
		/* if the pixel is turned on turn it off */
		} else {
			setpixel((pixel) {ant.x, ant.y}, 0);
			ant.vector_index = (ant.vector_index + 1u) % 4u; // turn right
		}

		/* move to next pixel, playing field is modeled after a torus */
		ant.x = (ant.x + xdcomp[ant.vector_index]) % UNUM_COLS;
		ant.y = (ant.y + ydcomp[ant.vector_index]) % UNUM_ROWS;

		wait(100);
	}
}
