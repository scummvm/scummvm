/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_SCREEN_H
#define XEEN_SCREEN_H

#include "common/rect.h"
#include "graphics/screen.h"
#include "xeen/font.h"
#include "xeen/sprites.h"

namespace Xeen {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class XeenEngine;

class Screen: public Graphics::Screen {
private:
	XeenEngine *_vm;
	byte _mainPalette[PALETTE_SIZE];
	byte _tempPalette[PALETTE_SIZE];
	XSurface _pages[2];
	XSurface _savedScreens[10];
	bool _fadeIn;

	/**
	 * Mark the entire screen for drawing
	 */
	void drawScreen();

	void fadeInner(int step);

	void updatePalette();

	void updatePalette(const byte *pal, int start, int count16);
public:
	Screen(XeenEngine *vm);
	virtual ~Screen() {}

	/**
	 * Base method that descendent classes can override for recording affected
	 * dirty areas of the surface
	 */
	virtual void addDirtyRect(const Common::Rect &r) { Graphics::Screen::addDirtyRect(r); }

	/**
	 * Load a palette resource into the temporary palette
	 */
	void loadPalette(const Common::String &name);

	/**
	 * Load a background resource into memory
	 */
	void loadBackground(const Common::String &name);

	/**
	 * Copy a loaded background into a display page
	 */
	void loadPage(int pageNum);

	void freePages();

	/**
	 * Merge the two pages along a horizontal split point
	 */
	void horizMerge(int xp = 0);

	/**
	 * Merge the two pages along a vertical split point
	 */
	void vertMerge(int yp);

	void draw(void *data = nullptr);

	void fadeIn(int step = 4);

	void fadeOut(int step = 4);

	void saveBackground(int slot = 1);

	void restoreBackground(int slot = 1);
};

} // End of namespace Xeen

#endif /* XEEN_SCREEN_H */
