/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Graphics maniuplation routines - private header file

#ifndef SAGA_GFX_H_
#define SAGA_GFX_H_

namespace Saga {

using Common::Point;
using Common::Rect;

struct R_CLIPINFO {
	// input members
	const Rect *src_rect;
	const Rect *dst_rect;
	const Point *dst_pt;

	// output members
	int nodraw;
	int src_draw_x;
	int src_draw_y;
	int dst_draw_x;
	int dst_draw_y;
	int draw_w;
	int draw_h;
};

struct PALENTRY {
	byte red;
	byte green;
	byte blue;
};

struct R_COLOR {
	int red;
	int green;
	int blue;
	int alpha;
};

struct R_SURFACE {
	byte *buf;
	int buf_w;
	int buf_h;
	int buf_pitch;
	Rect clip_rect;
};

#define R_PAL_ENTRIES 256

#define R_RGB_RED   0x00FF0000UL
#define R_RGB_GREEN 0x0000FF00UL
#define R_RGB_BLUE  0x000000FFUL

#define R_CURSOR_W 7
#define R_CURSOR_H 7

#define R_CURSOR_ORIGIN_X 4
#define R_CURSOR_ORIGIN_Y 4

#define R_RED_WEIGHT 0.299
#define R_GREEN_WEIGHT 0.587
#define R_BLUE_WEIGHT 0.114

class Gfx {
public:
	int simpleBlit(R_SURFACE *dst_s, R_SURFACE *src_s);
	int drawPalette(R_SURFACE *dst_s);
	int bufToSurface(R_SURFACE *ds, const byte *src, int src_w, int src_h, Rect *src_rect, Point *dst_pt);
	int bufToBuffer(byte * dst_buf, int dst_w, int dst_h, const byte *src,
		int src_w, int src_h, Rect *src_rect, Point *dst_pt);
	int drawRect(R_SURFACE *ds, Rect *dst_rect, int color);
	int drawFrame(R_SURFACE *ds, Point *p1, Point *p2, int color);
	int drawPolyLine(R_SURFACE *ds, Point *pts, int pt_ct, int draw_color);
	int getClipInfo(R_CLIPINFO *clipinfo);
	int clipLine(R_SURFACE *ds, const Point *src_p1, const Point *src_p2, Point *dst_p1, Point *dst_p2);
	void drawLine(R_SURFACE * ds, Point *p1, Point *p2, int color);

	Gfx(OSystem *system, int width, int height);
	R_SURFACE *getBackBuffer();
	int getWhite();
	int getBlack();
	int matchColor(unsigned long colormask);
	int setPalette(R_SURFACE *surface, PALENTRY *pal);
	int getCurrentPal(PALENTRY *src_pal);
	int palToBlack(R_SURFACE *surface, PALENTRY *src_pal, double percent);
	int blackToPal(R_SURFACE *surface, PALENTRY *src_pal, double percent);
	bool hitTestPoly(Point *points, unsigned int npoints, Point test_point);

private:
	void setCursor(int best_white);
	int _init;
	R_SURFACE _back_buf;
	int _white_index;
	int _black_index;
	byte _cur_pal[R_PAL_ENTRIES * 4];
	OSystem *_system;
};

} // End of namespace Saga

#endif
