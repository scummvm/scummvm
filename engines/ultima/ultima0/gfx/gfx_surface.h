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

#ifndef ULTIMA0_GFX_SURFACE_H
#define ULTIMA0_GFX_SURFACE_H

#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "ultima/ultima0/gfx/font.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

class GfxSurface : public Graphics::ManagedSurface {
private:
	Common::Point _textPos;
	byte _textColor = C_TEXT_DEFAULT;

	void newLine();

public:
	GfxSurface() : Graphics::ManagedSurface() {}
	GfxSurface(Graphics::ManagedSurface &surf, const Common::Rect &bounds) : Graphics::ManagedSurface(surf, bounds) {}

	/**
	 * Write some text to the surface
	 */
	void writeString(const Common::Point &pt, const Common::String &str,
		Graphics::TextAlign align = Graphics::kTextAlignLeft);
	void writeString(const Common::String &str, Graphics::TextAlign align = Graphics::kTextAlignLeft);
	void writeChar(uint32 chr);

	void setTextPos(const Common::Point &pt);
	byte setColor(byte color);
	byte setColor(byte r, byte g, byte b);
};

} // namespace Gfx
} // namespace Ultima0
} // namespace Ultima

#endif
