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

#include "graphics/primitives.h"
#include "graphics/managed_surface.h"
#include "dgds/drawing.h"

namespace Dgds {

namespace Drawing {

void drawPixel(int x, int y, int color, void *data) {
	Graphics::ManagedSurface *surface = (Graphics::ManagedSurface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h)
		*((byte *)surface->getBasePtr(x, y)) = (byte)color;
}

void filledCircle(int x, int y, int xr, int yr, Graphics::ManagedSurface *dst, byte fgcol, byte bgcol) {
	Graphics::drawEllipse(x - xr, y - yr, x + xr, y + yr, bgcol, true, drawPixel, dst);
	Graphics::drawEllipse(x - xr, y - yr, x + xr, y + yr, fgcol, false, drawPixel, dst);
}

void emptyCircle(int x, int y, int xr, int yr, Graphics::ManagedSurface *dst, byte fgcol) {
	Graphics::drawEllipse(x - xr, y - yr, x + xr, y + yr, fgcol, false, drawPixel, dst);
}

void rectClipped(const Common::Rect &r, const Common::Rect &clip, Graphics::ManagedSurface *dst, byte color) {
	Common::Rect clippedR(r);
	clippedR.clip(clip);
	if (clippedR.isEmpty())
		return;

	if (r.top >= clip.top && r.top < clip.bottom)
		dst->hLine(MAX(r.left, clip.left), r.top, MIN(r.right - 1, clip.right - 1), color);

	if (r.bottom - 1 >= clip.top && r.bottom - 1 < clip.bottom)
		dst->hLine(MAX(r.left, clip.left), r.bottom - 1, MIN(r.right - 1, clip.right - 1), color);

	if (r.left >= clip.left && r.left < clip.right)
		dst->vLine(r.left, MAX(r.top, clip.top), MIN(r.bottom - 1, clip.bottom - 1), color);

	if (r.right - 1 >= clip.left && r.right - 1 < clip.right)
		dst->vLine(r.right - 1, MAX(r.top, clip.top), MIN(r.bottom - 1, clip.bottom - 1), color);
}

}

} // end namespace Dgds
