#ifndef __cplusplus
#error NOTE: This file MUST be compiled as C++. In Visual C++, use the /Tp command line option.
#endif

extern "C" {
#include <gfx_system.h>
#include <gfx_tools.h>
};

#define LINEMACRO_1(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod) \
   x = (startx); y = (starty); \
   incrNE = ((deltalinear) > 0)? (deltalinear) : -(deltalinear); \
   incrNE <<= 1; \
   deltanonlinear <<= 1; \
   incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
   d = nonlinearstart-1;  \
   while (linearvar != (linearend)) { \
     buffer[linewidth * y + x] = color; \
     linearvar += linearmod; \
     if ((d+=incrE) < 0) { \
       d += incrNE; \
       nonlinearvar += nonlinearmod; \
     }; \
   }; \
  buffer[linewidth * y + x] = color;

#define LINEMACRO_N(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod) \
   x = (startx); y = (starty); \
   incrNE = ((deltalinear) > 0)? (deltalinear) : -(deltalinear); \
   incrNE <<= 1; \
   deltanonlinear <<= 1; \
   incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
   d = nonlinearstart-1;  \
   while (linearvar != (linearend)) { \
     memcpy(&buffer[linewidth * y + bpp*x],&color,bpp); \
     linearvar += linearmod; \
     if ((d+=incrE) < 0) { \
       d += incrNE; \
       nonlinearvar += nonlinearmod; \
     }; \
   }; \
  memcpy(&buffer[linewidth * y + x],&color,bpp);

void _dd_draw_line_buffer_1(byte *buffer, int linewidth, rect_t line, int color)
{
  /*void dither_line(picture_t buffers, int curx, int cury, short x1, short y1,
    int col1, int col2, int priority, int special, char drawenable)*/

  int dx, dy, incrE, incrNE, d, finalx, finaly;
  int x = line.x;
  int y = line.y;
  dx = line.xl;
  dy = line.yl;
  finalx = x + dx;
  finaly = y + dy;

  dx = abs(dx);
  dy = abs(dy);

  if (dx > dy) {
    if (finalx < x) {
      if (finaly < y) { /* llu == left-left-up */
	LINEMACRO_1(x, y, dx, dy, x, y, finalx, dx, -1, -1);
      } else {         /* lld */
	LINEMACRO_1(x, y, dx, dy, x, y, finalx, dx, -1, 1);
      }
    } else { /* x1 >= x */
      if (finaly < y) { /* rru */
	LINEMACRO_1(x, y, dx, dy, x, y, finalx, dx, 1, -1);
      } else {         /* rrd */
	LINEMACRO_1(x, y, dx, dy, x, y, finalx, dx, 1, 1);
      }
    }
  } else { /* dx <= dy */
    if (finaly < y) {
      if (finalx < x) { /* luu */
	LINEMACRO_1(x, y, dy, dx, y, x, finaly, dy, -1, -1);
      } else {         /* ruu */
	LINEMACRO_1(x, y, dy, dx, y, x, finaly, dy, -1, 1);
      }
    } else { /* y1 >= y */
      if (finalx < x) { /* ldd */
	LINEMACRO_1(x, y, dy, dx, y, x, finaly, dy, 1, -1);
      } else {         /* rdd */
	LINEMACRO_1(x, y, dy, dx, y, x, finaly, dy, 1, 1);
      }
    }
  }
}

void _dd_draw_line_buffer_n(int bpp,byte *buffer, int linewidth, rect_t line, byte * color)
{
  /*void dither_line(picture_t buffers, int curx, int cury, short x1, short y1,
    int col1, int col2, int priority, int special, char drawenable)*/

  int dx, dy, incrE, incrNE, d, finalx, finaly;
  int x = line.x;
  int y = line.y;
  dx = line.xl;
  dy = line.yl;
  finalx = x + dx;
  finaly = y + dy;

  dx = abs(dx);
  dy = abs(dy);

  if (dx > dy) {
    if (finalx < x) {
      if (finaly < y) { /* llu == left-left-up */
	LINEMACRO_N(x, y, dx, dy, x, y, finalx, dx, -1, -1);
      } else {         /* lld */
	LINEMACRO_N(x, y, dx, dy, x, y, finalx, dx, -1, 1);
      }
    } else { /* x1 >= x */
      if (finaly < y) { /* rru */
	LINEMACRO_N(x, y, dx, dy, x, y, finalx, dx, 1, -1);
      } else {         /* rrd */
	LINEMACRO_N(x, y, dx, dy, x, y, finalx, dx, 1, 1);
      }
    }
  } else { /* dx <= dy */
    if (finaly < y) {
      if (finalx < x) { /* luu */
	LINEMACRO_N(x, y, dy, dx, y, x, finaly, dy, -1, -1);
      } else {         /* ruu */
	LINEMACRO_N(x, y, dy, dx, y, x, finaly, dy, -1, 1);
      }
    } else { /* y1 >= y */
      if (finalx < x) { /* ldd */
	LINEMACRO_N(x, y, dy, dx, y, x, finaly, dy, 1, -1);
      } else {         /* rdd */
	LINEMACRO_N(x, y, dy, dx, y, x, finaly, dy, 1, 1);
      }
    }
  }
}

void _dd_draw_line(gfx_mode_t * mode, rect_t line, byte * dest, int linewidth,gfx_color_t color)
{
	byte ca[4];
	unsigned short co1;

	switch(mode->bytespp)
	{
		case 1:
			_dd_draw_line_buffer_1(dest,linewidth,line,color.visual.global_index);
			break;
		case 2:
			co1  = (color.visual.r << mode->red_shift) & mode->red_mask;
			co1 |= (color.visual.g << mode->green_shift) & mode->green_mask;
			co1 |= (color.visual.b << mode->blue_shift) & mode->blue_mask;
			_dd_draw_line_buffer_n(2,dest,linewidth,line, (byte *) &co1);
			break;
		case 3:
			ca[mode->red_shift/8]=color.visual.r;
			ca[mode->green_shift/8]=color.visual.g;
			ca[mode->blue_shift/8]=color.visual.b;
			_dd_draw_line_buffer_n(3,dest,linewidth,line, ca);
			break;
	}
}
