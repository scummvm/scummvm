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

#ifndef DGDS_DRAWING_H
#define DGDS_DRAWING_H

namespace Graphics {
	class ManagedSurface;
}

namespace Dgds {

namespace Drawing {

	// A function that can be used as the callback for Graphics::Primitives functions.
	void drawPixel(int x, int y, int color, void *data);

	void filledCircle(int x, int y, int xr, int yr, Graphics::ManagedSurface *dst, byte fgcol, byte bgcol);
	void emptyCircle(int x, int y, int xr, int yr, Graphics::ManagedSurface *dst, byte fgcol);

	/**
	 * A non-filled rectangle but don't draw outside clipWin.
	 *
	 * Applies regular rect rounding rules and draws lines at right-1 and bottom-1, so that
	 * clipRect == rect will draw a complete rectangle
	 */
	void rectClipped(const Common::Rect &rect, const Common::Rect &clipWin, Graphics::ManagedSurface *dst, byte color);
}

} // end namespace Dgds

#endif // DGDS_DRAWING_H
