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

#include "got/gfx/gfx_surface.h"
#include "got/vars.h"

namespace Got {
namespace Gfx {

void GfxSurface::print(const Common::Point &pos,
					   const Common::String &text, int color) {
	_G(font).drawString(this, pos, text, color);
}

void GfxSurface::printChar(uint32 chr, int x, int y, uint32 color) {
	_G(font).drawChar(this, chr, x, y, color);
}

void GfxSurface::rawPrintChar(uint32 chr, int x, int y, uint32 color) {
	_G(font).rawDrawChar(surfacePtr(), chr, x, y, color);
}

} // namespace Gfx
} // namespace Got
