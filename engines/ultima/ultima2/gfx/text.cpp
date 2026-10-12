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


#include "ultima/ultima2/gfx/text.h"
#include "ultima/ultima2/data/data.h"
#include "ultima/ultima2/data/tiles.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

// White and magenta, within the original 4-color CGA palette
constexpr byte COLOR_WHITE = 3;
constexpr byte COLOR_MAGENTA = 2;

// Where the table of colors of the text starts within the colors of an EGA theme
constexpr int EGA_TEXT_COLORS = 8;

// Magenta, within the composite colors
constexpr int COMPOSITE_MAGENTA = 10;

byte textColor(TextColor color) {
	if (!g_engine->_mixedCaseText)
		return (color == TEXT_HIGHLIGHT) ? COLOR_MAGENTA : COLOR_WHITE;

	const Data::RenderMode mode = g_engine->getRenderMode();
	if (Data::isEgaMode(mode))
		return Data::EGA_PALETTE_BASE + Data::egaColors(mode)[EGA_TEXT_COLORS + color];

	// The CGA drivers have everything white apart from highlighted text
	if (color == TEXT_HIGHLIGHT) {
		return (mode == Data::RENDER_CGA_COMPOSITE) ?
			Data::COMPOSITE_PALETTE_BASE + COMPOSITE_MAGENTA : COLOR_MAGENTA;
	}

	return COLOR_WHITE;
}

void setReverseText(Graphics::Views::GfxSurface &s, bool on) {
	if (on)
		s.setColor(0, textColor(TEXT_REVERSE));
	else
		s.setColor(textColor(TEXT_NORMAL), 0);
}

void TextFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (!g_engine->_mixedCaseText && chr >= 'a' && chr <= 'z')
		chr -= 'a' - 'A';

	DosFont::drawChar(dst, chr, x, y, color);
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
