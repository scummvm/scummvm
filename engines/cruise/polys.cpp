/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "cruise/cruise_main.h"
#include "common/util.h"

namespace Cruise {

#include <stdlib.h>
#include <math.h>

typedef char ColorP;

#define SCREENHEIGHT 200
#define MAXPTS 10
#define putdot(x,y) {if ((y >= 0) && (y < SCREENHEIGHT)) dots[y][counters[y]++] = x;}

void hline(int x1, int x2, int y, char c) {
	for (; x1 <= x2; x1++) {
		pixel(x1, y, c);
	}
}

void vline(int x, int y1, int y2, char c) {
	for (; y1 <= y2; y1++) {
		pixel(x, y1, c);
	}
}

void bsubline_1(int x1, int y1, int x2, int y2, char c) {
	int x, y, ddx, ddy, e;
	ddx = abs(x2 - x1);
	ddy = abs(y2 - y1) << 1;
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (x = x1, y = y1; x <= x2; x++) {

		pixel(x, y, c);
		if (e < 0) {
			y++;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}

}

void bsubline_2(int x1, int y1, int x2, int y2, char c) {

	int x, y, ddx, ddy, e;
	ddx = abs(x2 - x1) << 1;
	ddy = abs(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (y = y1, x = x1; y <= y2; y++) {

		pixel(x, y, c);
		if (e < 0) {
			x++;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}

}

void bsubline_3(int x1, int y1, int x2, int y2, char c) {

	int x, y, ddx, ddy, e;

	ddx = abs(x1 - x2) << 1;
	ddy = abs(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (y = y1, x = x1; y <= y2; y++) {

		pixel(x, y, c);
		if (e < 0) {
			x--;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}

}

void bsubline_4(int x1, int y1, int x2, int y2, char c) {

	int x, y, ddx, ddy, e;

	ddy = abs(y2 - y1) << 1;
	ddx = abs(x1 - x2);
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (x = x1, y = y1; x <= x2; x++) {

		pixel(x, y, c);
		if (e < 0) {
			y--;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}
}

void line(int x1, int y1, int x2, int y2, char c) {

	float k;

	if ((x1 == x2) && (y1 == y2)) {
		pixel(x1, y1, c);
		return;
	}

	if (x1 == x2) {
		vline(x1, MIN(y1, y2), MAX(y1, y2), c);
		return;
	}

	if (y1 == y2) {
		hline(MIN(x1, x2), MAX(x1, x2), y1, c);
		return;
	}

	k = (float)(y2 - y1) / (float)(x2 - x1);

	if ((k >= 0) && (k <= 1)) {
		bsubline_1(x1, y1, x2, y2, c);
	} else if (k > 1) {
		bsubline_2(x1, y1, x2, y2, c);
	} else if ((k < 0) && (k >= -1)) {
		bsubline_4(x1, y1, x2, y2, c);
	} else {
		bsubline_3(x1, y1, x2, y2, c);
	}
}

void fillpoly(short int *datas, int lineCount, ColorP color) {
	static int dots[SCREENHEIGHT][MAXPTS];
	static int counters[SCREENHEIGHT];
	short int x1, y1, x2, y2;
	int i, j, k, dir = -2;
	double step, curx;

	switch (lineCount) {
	case 0:		// do nothing
		return;
	case 1:		// draw pixel
		pixel(datas[0], datas[1], color);
		return;
	case 2:		// draw line
		line(datas[0], datas[1], datas[2], datas[3], color);
		return;
	default:		// go on and draw polygon
		break;
	}

	// Reinit array counters

	for (i = 0; i < SCREENHEIGHT; i++) {
		counters[i] = 0;
	}
	// Drawing lines

	x2 = datas[lineCount * 2 - 2];
	y2 = datas[lineCount * 2 - 1];

	for (i = 0; i < lineCount; i++) {
		x1 = x2;
		y1 = y2;
		x2 = datas[i * 2];
		y2 = datas[i * 2 + 1];

		//  line(x1, y1, x2, y2, color);
		//  continue;

		if (y1 == y2) {
			//      printf("Horizontal line. x1: %i, y1: %i, x2: %i, y2: %i\n", x1, y1, x2, y2);
			if (dir) {
				putdot(x1, y1);
				dir = 0;
			}
		} else {
			step = (double)(x2 - x1) / (y2 - y1);

			//  printf("x1: %i, y1 = %i, x2 = %i, y2 = %i, step: %f\n", x1, y1, x2, y2, step);

			curx = x1;

			if (y1 < y2) {
				for (j = y1; j < y2; j++, curx += step) {
					//    printf("j = %i, curx = %f\n", j, curx);
					putdot((int)(curx + 0.5), j);
				}
				if (dir == -1) {
					//    printf("Adding extra (%i, %i)\n", x1, y1);
					putdot(x1, y1);
				}
				dir = 1;
			} else {
				for (j = y1; j > y2; j--, curx -= step) {
					//    printf("j = %i, curx = %f\n", j, curx);
					putdot((int)(curx + 0.5), j);
				}
				if (dir == 1) {
					//    printf("Adding extra (%i, %i)\n", x1, y1);
					putdot(x1, y1);
				}
				dir = -1;
			}
		}
	}

	x1 = x2;
	y1 = y2;
	x2 = datas[0];
	y2 = datas[1];

	if (((y1 < y2) && (dir == -1)) || ((y1 > y2) && (dir == 1))
	    || ((y1 == y2) && (dir == 0))) {
		//  printf("Adding final extra (%i, %i)\n", x1, y1);
		putdot(x1, y1);
	}
	// NOTE: all counters should be even now. If not, this is a bad (color) thing :-P

	// Sorting datas

	for (i = 0; i < SCREENHEIGHT; i++) {
		// Very bad sorting... but arrays are very small (0, 2 or 4), so it's no quite use...
		for (j = 0; j < (counters[i] - 1); j++) {
			for (k = 0; k < (counters[i] - 1); k++) {
				if (dots[i][k] > dots[i][k + 1]) {
					int temp;
					temp = dots[i][k];
					dots[i][k] = dots[i][k + 1];
					dots[i][k + 1] = temp;
				}
			}
		}
	}

	// Drawing.

	for (i = 0; i < SCREENHEIGHT; i++) {
		if (counters[i]) {
			//      printf("%i dots on line %i\n", counters[i], i);
			for (j = 0; j < counters[i] - 1; j += 2) {
				//    printf("Drawing line (%i, %i)-%i\n", dots[i][j], dots[i][j + 1], i);
				hline(dots[i][j], dots[i][j + 1], i, color);
#ifdef DEBUGGING_POLYS
				if ((!dots[i][j]) || !(dots[i][j + 1])) {
					printf("fillpoly: BLARGH!\n");
					exit(-1);
				}
#endif
			}
		}
	}
}

} // End of namespace Cruise
