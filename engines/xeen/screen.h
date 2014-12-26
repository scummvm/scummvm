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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/array.h"
#include "common/rect.h"
#include "xeen/xsurface.h"

namespace Xeen {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class XeenEngine;

class Screen: public XSurface {
private:
	XeenEngine *_vm;
	Common::List<Common::Rect> _dirtyRects;
	byte _mainPalette[PALETTE_SIZE];
	byte _tempPaltte[PALETTE_SIZE];
	XSurface _pages[2];
	bool _fadeMode;

	void mergeDirtyRects();

	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);

	void drawScreen();

	void fadeInner(int step);

	void updatePalette();

	void updatePalette(const byte *pal, int start, int count16);
public:
	virtual void addDirtyRect(const Common::Rect &r);
public:
	Screen(XeenEngine *vm);

	void update();

	void loadPalette(const Common::String &name);

	void loadBackground(const Common::String &name);

	void loadPage(int pageNum);

	void horizMerge(int xp);

	void vertMerge(int yp);

	void draw(void *data = nullptr);

	void fade(int step);

	void fade2(int step);
};

} // End of namespace Xeen

#endif /* XEEN_SCREEN_H */
