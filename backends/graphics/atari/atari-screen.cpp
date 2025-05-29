/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "atari-screen.h"

#include <mint/falcon.h>

#include "atari-graphics.h"		// MAX_HZ_SHAKE, MAX_V_SHAKE
#include "atari-supervidel.h"	// g_hasSuperVidel
//#include "backends/platform/atari/atari-debug.h"

Screen::Screen(bool tt, int width, int height, const Graphics::PixelFormat &format, const Palette *palette_)
	: palette(palette_)
	, _tt(tt) {

#ifdef USE_SUPERVIDEL
	if (g_hasSuperVidel) {
		surf.reset(new SuperVidelSurface(
			width + 2 * MAX_HZ_SHAKE,
			height + 2 * MAX_V_SHAKE,
			format));
		_offsettedSurf.reset(new SuperVidelSurface());
	} else
#endif
	{
		surf.reset(new AtariSurface(
			width + (_tt ? 0 : 2 * MAX_HZ_SHAKE),
			height + 2 * MAX_V_SHAKE,
			format));
		_offsettedSurf.reset(new AtariSurface());
	}

	_offsettedSurf->create(
		*surf,
		Common::Rect(
			Common::Point(
				(surf->w - width) / 2,		// left
				(surf->h - height) / 2),	// top
			width, height));
}

void Screen::reset(int width, int height, const Graphics::Surface &boundingSurf) {
	clearDirtyRects();

	rez = -1;
	mode = -1;

	const int bitsPerPixel = surf->getBitsPerPixel();

	// erase old screen
	_offsettedSurf->fillRect(_offsettedSurf->getBounds(), 0);

	if (_tt) {
		if (width <= 320 && height <= 240) {
			surf->w = 320;
			surf->h = 240 + 2 * MAX_V_SHAKE;
			surf->pitch = 2 * surf->w * bitsPerPixel / 8;
			rez = kRezValueTTLow;
		} else {
			surf->w = 640;
			surf->h = 480 + 2 * MAX_V_SHAKE;
			surf->pitch = surf->w * bitsPerPixel / 8;
			rez = kRezValueTTMid;
		}
	} else {
		mode = VsetMode(VM_INQUIRE) & PAL;

		if (VgetMonitor() == MON_VGA) {
			mode |= VGA | (bitsPerPixel == 4 ? BPS4 : (g_hasSuperVidel ? BPS8C : BPS8));

			if (width <= 320 && height <= 240) {
				surf->w = 320;
				surf->h = 240;
				mode |= VERTFLAG | COL40;
			} else {
				surf->w = 640;
				surf->h = 480;
				mode |= COL80;
			}
		} else {
			mode |= TV | (bitsPerPixel == 4 ? BPS4 : BPS8);

			if (width <= 320 && height <= 200) {
				surf->w = 320;
				surf->h = 200;
				mode |= COL40;
			} else if (width <= 320*1.2 && height <= 200*1.2) {
				surf->w = 320*1.2;
				surf->h = 200*1.2;
				mode |= OVERSCAN | COL40;
			} else if (width <= 640 && height <= 400) {
				surf->w = 640;
				surf->h = 400;
				mode |= VERTFLAG | COL80;
			} else {
				surf->w = 640*1.2;
				surf->h = 400*1.2;
				mode |= VERTFLAG | OVERSCAN | COL80;
			}
		}

		surf->w += 2 * MAX_HZ_SHAKE;
		surf->h += 2 * MAX_V_SHAKE;
		surf->pitch = surf->w * bitsPerPixel / 8;
	}

	_offsettedSurf->create(
		*surf,
		Common::Rect(
			Common::Point(
				(surf->w - width) / 2,		// left
				(surf->h - height) / 2),	// top
			width, height));

	cursor.reset(_offsettedSurf.get(), &boundingSurf);
	cursor.setPosition(boundingSurf.w / 2, boundingSurf.h / 2);
}

void Screen::addDirtyRect(const Graphics::Surface &srcSurface, int x, int y, int w, int h, bool directRendering) {
	if (fullRedraw)
		return;

	if ((w == srcSurface.w && h == srcSurface.h)
		|| dirtyRects.size() == 128) {	// 320x200 can hold at most 250 16x16 rectangles
		//atari_debug("addDirtyRect[%d]: purge %d x %d", (int)dirtyRects.size(), srcSurface.w, srcSurface.h);

		dirtyRects.clear();
		// even if srcSurface.w != _offsettedSurf.w, alignRect would lead to the same result
		dirtyRects.insert(_offsettedSurf->getBounds());

		cursor.reset(_offsettedSurf.get(), &srcSurface);

		fullRedraw = true;
	} else {
		// x,y are relative to srcSurface but screen's width is always aligned to 16 bytes
		// so both dirty rects and cursor must be drawn in screen coordinates
		const int xOffset = (_offsettedSurf->w - srcSurface.w) / 2;

		const Common::Rect alignedRect = AtariSurface::alignRect(x + xOffset, y, x + xOffset + w, y + h);

		dirtyRects.insert(alignedRect);

		// Check whether the cursor background intersects the dirty rect. Has to be done here,
		// before the actual drawing (especially in case of direct rendering). There's one more
		// check in AtariGraphicsManager::updateScreenInternal for the case when there are no
		// dirty rectangles but the cursor itself has changed.
		const Common::Rect cursorBackgroundRect = cursor.flushBackground(alignedRect, directRendering);
		if (!cursorBackgroundRect.isEmpty()) {
			dirtyRects.insert(cursorBackgroundRect);
		}
	}
}
