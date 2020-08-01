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

#include "common/system.h"
#include "common/algorithm.h"
#include "graphics/screen.h"
#include "graphics/palette.h"

namespace Graphics {

Screen::Screen(): ManagedSurface() {
	create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
}

Screen::Screen(int width, int height): ManagedSurface() {
	create(width, height);
}

Screen::Screen(int width, int height, PixelFormat pixelFormat): ManagedSurface() {
	create(width, height, pixelFormat);
}

void Screen::update() {
	// Merge the dirty rects
	mergeDirtyRects();

	// Loop through copying dirty areas to the physical screen
	Common::List<Common::Rect>::iterator i;
	for (i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i) {
		const Common::Rect &r = *i;
		const byte *srcP = (const byte *)getBasePtr(r.left, r.top);
		g_system->copyRectToScreen(srcP, pitch, r.left, r.top,
			r.width(), r.height());
	}

	// Signal the physical screen to update
	updateScreen();
	_dirtyRects.clear();
}

void Screen::updateScreen() {
	// Update the screen
	g_system->updateScreen();
}

void Screen::addDirtyRect(const Common::Rect &r) {
	Common::Rect bounds = r;
	bounds.clip(getBounds());
	bounds.translate(getOffsetFromOwner().x, getOffsetFromOwner().y);

	if (bounds.width() > 0 && bounds.height() > 0)
		_dirtyRects.push_back(bounds);
}

void Screen::makeAllDirty() {
	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void Screen::mergeDirtyRects() {
	Common::List<Common::Rect>::iterator rOuter, rInner;

	// Process the dirty rect list to find any rects to merge
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {

			if ((*rOuter).intersects(*rInner)) {
				// These two rectangles overlap, so merge them
				unionRectangle(*rOuter, *rOuter, *rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

bool Screen::unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2) {
	destRect = src1;
	destRect.extend(src2);

	return !destRect.isEmpty();
}

void Screen::getPalette(byte palette[PALETTE_SIZE]) {
	assert(format.bytesPerPixel == 1);
	g_system->getPaletteManager()->grabPalette(palette, 0, PALETTE_COUNT);
}

void Screen::getPalette(byte *palette, uint start, uint num) {
	assert(format.bytesPerPixel == 1);
	g_system->getPaletteManager()->grabPalette(palette, start, num);
}

void Screen::setPalette(const byte palette[PALETTE_SIZE]) {
	assert(format.bytesPerPixel == 1);
	g_system->getPaletteManager()->setPalette(palette, 0, PALETTE_COUNT);
}

void Screen::setPalette(const byte *palette, uint start, uint num) {
	assert(format.bytesPerPixel == 1);
	g_system->getPaletteManager()->setPalette(palette, start, num);
	ManagedSurface::setPalette(palette, start, num);
}

void Screen::clearPalette() {
	byte palette[PALETTE_SIZE];
	Common::fill(&palette[0], &palette[PALETTE_SIZE], 0);
	setPalette(palette);
}

} // End of namespace Graphics
