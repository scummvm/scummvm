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

#ifndef ULTIMA1_GFX_FONT_H
#define ULTIMA1_GFX_FONT_H

#include "graphics/fonts/dosfont.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

/**
 * Derived DosFont variant - the left ('\x10') and right ('\x11') characters
 * will always be drawn as blue solids with a white outline
 */
class Font : public Graphics::DosFont {
public:
	~Font() override {
	}

	/**
	 * Draw a character at a specific point on the surface.
	 */
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima

#endif
