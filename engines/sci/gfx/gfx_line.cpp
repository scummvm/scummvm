/***************************************************************************
 gfx_line.c Copyright (C) 2000 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#define LINEMACRO(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod) \
   x = (startx); y = (starty); \
   incrNE = ((deltalinear) > 0)? (deltalinear) : -(deltalinear); \
   incrNE <<= 1; \
   deltanonlinear <<= 1; \
   incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
   d = nonlinearstart-1;  \
   while (linearvar != (linearend)) { \
     memcpy(buffer + linewidth * y + x, &color, PIXELWIDTH); \
     linearvar += linearmod; \
     if ((d+=incrE) < 0) { \
       d += incrNE; \
       nonlinearvar += nonlinearmod; \
     }; \
   }; \
   memcpy(buffer + linewidth * y + x, &color, PIXELWIDTH);


static inline
void DRAWLINE_FUNC(byte *buffer, int linewidth, point_t start, point_t end, unsigned int color) {
	int dx, dy, incrE, incrNE, d, finalx, finaly;
	int x = start.x;
	int y = start.y;
	dx = end.x - start.x;
	dy = end.y - start.y;
	finalx = end.x;
	finaly = end.y;
#ifdef WORDS_BIGENDIAN
	color = GUINT32_SWAP_LE_BE_CONSTANT(color);
#endif
	dx = abs(dx);
	dy = abs(dy);

	if (dx > dy) {
		if (finalx < x) {
			if (finaly < y) { /* llu == left-left-up */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -PIXELWIDTH, -1);
			} else {         /* lld */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -PIXELWIDTH, 1);
			}
		} else { /* x1 >= x */
			if (finaly < y) { /* rru */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, PIXELWIDTH, -1);
			} else {         /* rrd */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, PIXELWIDTH, 1);
			}
		}
	} else { /* dx <= dy */
		if (finaly < y) {
			if (finalx < x) { /* luu */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, -PIXELWIDTH);
			} else {         /* ruu */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, PIXELWIDTH);
			}
		} else { /* y1 >= y */
			if (finalx < x) { /* ldd */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, -PIXELWIDTH);
			} else {         /* rdd */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, PIXELWIDTH);
			}
		}
	}
}



#undef LINEMACRO
