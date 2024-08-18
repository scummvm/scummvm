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

}

} // end namespace Dgds
