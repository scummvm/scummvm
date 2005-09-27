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

struct ClipData {
	// input members
	Rect sourceRect;
	Rect destRect;
	Point destPoint;

	// output members
	Point drawSource;
	Point drawDest;
	int drawWidth;
	int drawHeight;

	bool calcClip() {
		Common::Rect s;

		// Adjust the rect to draw to its screen coordinates
		s = sourceRect;
		s.left += destPoint.x;
		s.right += destPoint.x;
		s.top += destPoint.y;
		s.bottom += destPoint.y;

		s.clip(destRect);

		if ((s.width() <= 0) || (s.height() <= 0)) {
			return false;
		}

		drawSource.x = s.left - sourceRect.left - destPoint.x;
		drawSource.y = s.top - sourceRect.top - destPoint.y;
		drawDest.x = s.left;
		drawDest.y = s.top;
		drawWidth = s.width();
		drawHeight = s.height();

		return true;
	}
};

#pragma START_PACK_STRUCTS
struct PalEntry {
	byte red;
	byte green;
	byte blue;
} GCC_PACK;

#pragma END_PACK_STRUCTS

struct Color {
	int red;
	int green;
	int blue;
	int alpha;
};

struct Surface : Graphics::Surface {

	void transitionDissolve(const byte *sourceBuffer, const Common::Rect &sourceRect, int flags, double percent);
	void drawPalette();
	void drawPolyLine(const Point *points, int count, int color);
	void blit(const Common::Rect &destRect, const byte *sourceBuffer);

	void getRect(Common::Rect &rect) {
		rect.left = rect.top = 0;
		rect.right = w;
		rect.bottom = h;
	}
	void drawFrame(const Common::Point &p1, const Common::Point &p2, int color) {
		Common::Rect rect(MIN(p1.x, p2.x), MIN(p1.y, p2.y), MAX(p1.x, p2.x) + 1, MAX(p1.y, p2.y) + 1);
		frameRect(rect, color);
	}
	void drawRect(const Common::Rect &destRect, int color) {
		Common::Rect rect(w , h);
		rect.clip(destRect);

		if (rect.isValidRect()) {
			fillRect(rect, color);
		}
	}
};

#define PAL_ENTRIES 256

#define CURSOR_W 7
#define CURSOR_H 7

#define CURSOR_ORIGIN_X 4
#define CURSOR_ORIGIN_Y 4

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point);
class SagaEngine;

class Gfx {
public:

	Gfx(SagaEngine *vm, OSystem *system, int width, int height, GameDetector &detector);
	~Gfx();
	Surface *getBackBuffer() {
		return &_backBuffer;
	}

	void setPalette(const PalEntry *pal);
	void getCurrentPal(PalEntry *src_pal);
	void palToBlack(PalEntry *src_pal, double percent);
	void blackToPal(PalEntry *src_pal, double percent);
	void updateCursor() { setCursor(); }
	void showCursor(bool state);

private:
	void setCursor();
	int _init;
	Surface _backBuffer;
	byte _currentPal[PAL_ENTRIES * 4];
	OSystem *_system;
	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
