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

#ifndef GOT_GFX_SURFACE_H
#define GOT_GFX_SURFACE_H

#include "graphics/managed_surface.h"

namespace Got {
namespace Gfx {

class GfxSurface : public Graphics::ManagedSurface {
public:
	GfxSurface() : Graphics::ManagedSurface() {}
	GfxSurface(Graphics::ManagedSurface &surf, const Common::Rect &bounds) : Graphics::ManagedSurface(surf, bounds) {}

	/**
     * Write some text to the surface
     */
	void print(const Common::Point &pos, const Common::String &text, int color);
	void printChar(uint32 chr, int x, int y, uint32 color);
	void rawPrintChar(uint32 chr, int x, int y, uint32 color);
};

} // namespace Gfx
} // namespace Got

#endif
