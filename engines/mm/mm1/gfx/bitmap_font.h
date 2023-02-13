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

#include "mm/shared/utils/bitmap_font.h"

namespace MM {
namespace MM1 {
namespace Gfx {

/**
 * The Might and Magic 1 font only has uppercase. So we
 * override the base BitmapFont class to uppercase characters
 */
class BitmapFont : public ::MM::BitmapFont {
public:
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override {
		chr = (chr & 0x80) | toupper(chr & 0x7f);
		MM::BitmapFont::drawChar(dst, chr, x, y, color);
	}

	void drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override {
		chr = (chr & 0x80) | toupper(chr & 0x7f);
		MM::BitmapFont::drawChar(dst, chr, x, y, color);
	}
};

} // namespace Gfx
} // namespace MM1
} // namespace MM
