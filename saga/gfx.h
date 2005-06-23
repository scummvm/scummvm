/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "graphics/surface.h"

namespace Saga {

using Common::Point;
using Common::Rect;

struct CLIPINFO {
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

struct COLOR {
	int red;
	int green;
	int blue;
	int alpha;
};

struct SURFACE : Graphics::Surface {
	Rect clip_rect;
};

#define PAL_ENTRIES 256

#define CURSOR_W 7
#define CURSOR_H 7

#define CURSOR_ORIGIN_X 4
#define CURSOR_ORIGIN_Y 4

int drawPalette(SURFACE *dst_s);
int bufToSurface(SURFACE *ds, const byte *src, int src_w, int src_h, Rect *src_rect, Point *dst_pt);
int bufToBuffer(byte * dst_buf, int dst_w, int dst_h, const byte *src,
	int src_w, int src_h, Rect *src_rect, Point *dst_pt);
int getClipInfo(CLIPINFO *clipinfo);
int drawRect(SURFACE *ds, Rect &dst_rect, int color);
int drawFrame(SURFACE *ds, const Point *p1, const Point *p2, int color);
int drawPolyLine(SURFACE *ds, const Point *pts, int pt_ct, int draw_color);
int clipLine(SURFACE *ds, const Point *src_p1, const Point *src_p2, Point *dst_p1, Point *dst_p2);

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point);

class Gfx {
public:

	Gfx(OSystem *system, int width, int height, GameDetector &detector);
	SURFACE *getBackBuffer();
	int setPalette(SURFACE *surface, PALENTRY *pal);
	int getCurrentPal(PALENTRY *src_pal);
	int palToBlack(SURFACE *surface, PALENTRY *src_pal, double percent);
	int blackToPal(SURFACE *surface, PALENTRY *src_pal, double percent);
	void updateCursor() { setCursor(); }
	void showCursor(bool state);

private:
	void setCursor();
	int _init;
	SURFACE _back_buf;
	byte _cur_pal[PAL_ENTRIES * 4];
	OSystem *_system;
};

} // End of namespace Saga

#endif
