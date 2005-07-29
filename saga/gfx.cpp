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

// Misc. graphics routines

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/interface.h"

#include "common/system.h"

namespace Saga {

Gfx::Gfx(SagaEngine *vm, OSystem *system, int width, int height, GameDetector &detector) : _vm(vm), _system(system) {
	_system->beginGFXTransaction();
		_vm->initCommonGFX(detector);
		_system->initSize(width, height);
	_system->endGFXTransaction();

	debug(5, "Init screen %dx%d", width, height);
	// Convert surface data to R surface data
	_backBuffer.create(width, height, 1);

	// Set module data
	_init = 1;

	// For now, always show the mouse cursor.
	setCursor();
	_system->showMouse(true);
}


Gfx::~Gfx() {
  _backBuffer.free();
}


void Surface::drawPalette() {
	int x;
	int y;
	int color = 0;
	Rect palRect;

	for (y = 0; y < 16; y++) {
		palRect.top = (y * 8) + 4;
		palRect.bottom = palRect.top + 8;

		for (x = 0; x < 16; x++) {
			palRect.left = (x * 8) + 4;
			palRect.right = palRect.left + 8;

			drawRect(palRect, color);
			color++;
		}
	}
}

// * Copies a rectangle from a raw 8 bit pixel buffer to the specified surface.
// - The surface must match the logical dimensions of the buffer exactly.
void Surface::blit(const Common::Rect &destRect, const byte *sourceBuffer) {
	const byte *readPointer;
	byte *writePointer;
	int row;
	ClipData clipData;

	clipData.sourceRect.left = 0;
	clipData.sourceRect.top = 0;
	clipData.sourceRect.right = destRect.width();
	clipData.sourceRect.bottom = destRect.height();

	clipData.destPoint.x = destRect.left;
	clipData.destPoint.y = destRect.top;
	clipData.destRect.left = 0;
	clipData.destRect.right = w;
	clipData.destRect.top = 0;
	clipData.destRect.bottom = h;

	if (!clipData.calcClip()) {
		return;
	}

	// Transfer buffer data to surface
	readPointer = (sourceBuffer + clipData.drawSource.x) +
						(clipData.sourceRect.right * clipData.drawSource.y);

	writePointer = ((byte *)pixels + clipData.drawDest.x) + (pitch * clipData.drawDest.y);

	for (row = 0; row < clipData.drawHeight; row++) {
		memcpy(writePointer, readPointer, clipData.drawWidth);

		writePointer += pitch;
		readPointer += clipData.sourceRect.right;
	}
}

void Surface::drawPolyLine(const Point *points, int count, int color) {
	int i;
	if (count >= 3) {
		for (i = 1; i < count; i++) {
			drawLine(points[i].x, points[i].y, points[i - 1].x, points[i - 1].y, color);
		}

		drawLine(points[count - 1].x, points[count - 1].y, points->x, points->y, color);
	}
}

/**
* Dissolve one image with another.
* If flags if set to 1, do zero masking.
*/
void Surface::transitionDissolve(const byte *sourceBuffer, const Common::Rect &sourceRect, int flags, double percent) {
#define XOR_MASK 0xB400;
	int pixelcount = w * h;
	int seqlimit = (int)(65535 * percent);
	int seq = 1;
	int i, x1, y1;
	byte color;

	for (i = 0; i < seqlimit; i++) {
		if (seq & 1) {
			seq = (seq >> 1) ^ XOR_MASK;
		} else {
			seq = seq >> 1;
		}

		if (seq == 1) {
			return;
		}

		if (seq >= pixelcount) {
			continue;
		} else {
			x1 = seq % w;
			y1 = seq / w;

			if (sourceRect.contains(x1, y1)) {
				color = sourceBuffer[(x1-sourceRect.left) + sourceRect.width()*(y1-sourceRect.top)];
				if (flags == 0 || color)
					((byte*)pixels)[seq] = color;
			}
		}
	}
}


void Gfx::setPalette(PalEntry *pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		ppal[0] = pal[i].red;
		ppal[1] = pal[i].green;
		ppal[2] = pal[i].blue;
		ppal[3] = 0;
	}

	_system->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::getCurrentPal(PalEntry *src_pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		src_pal[i].red = ppal[0];
		src_pal[i].green = ppal[1];
		src_pal[i].blue = ppal[2];
	}
}

void Gfx::palToBlack(PalEntry *src_pal, double percent) {
	int i;
	//int fade_max = 255;
	int new_entry;
	byte *ppal;

	double fpercent;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	_system->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::blackToPal(PalEntry *src_pal, double percent) {
	int new_entry;
	double fpercent;
	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;
	byte *ppal;
	int i;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red - src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green - src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue - src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	// Find the best white and black color indices again
	if (percent >= 1.0) {
		for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
			color_delta = ppal[0];
			color_delta += ppal[1];
			color_delta += ppal[2];

			if (color_delta < best_bdelta) {
				best_bindex = i;
				best_bdelta = color_delta;
			}

			if (color_delta > best_wdelta) {
				best_windex = i;
				best_wdelta = color_delta;
			}
		}
	}

	_system->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::showCursor(bool state) {
	updateCursor();
	g_system->showMouse(state);
}

void Gfx::setCursor() {
	// Set up the mouse cursor
	const byte A = kITEColorLightGrey;
	const byte B = kITEColorWhite;

	const byte cursor_img[CURSOR_W * CURSOR_H] = {
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		A, A, A, B, A, A, A,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
	};

	_system->setMouseCursor(cursor_img, CURSOR_W, CURSOR_H, 3, 3, 0);
}

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point) {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	const Point *vtx0 = &points[npoints - 1];
	const Point *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);
	for (pt = 0; pt < npoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= test_point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

} // End of namespace Saga
