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

#include "mm/mm1/views_enh/scroll_view.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define SYMBOL_WIDTH FRAME_BORDER_SIZE
#define SYMBOL_HEIGHT FRAME_BORDER_SIZE

ScrollView::ScrollView(const Common::String &name) :
	TextView(name, g_engine) {
}

ScrollView::ScrollView(const Common::String &name,
		UIElement *owner) : TextView(name, owner) {
}

void ScrollView::draw() {
	frame();
	fill();
	setTextColor(0);
}

void ScrollView::frame() {
	int xCount = (_bounds.width() - 9) / SYMBOL_WIDTH;
	int yCount = (_bounds.height() - 9) / SYMBOL_HEIGHT;

	// Write the top line
	_symbolPos = Common::Point(0, 0);
	writeSymbol(0);

	if (xCount > 0) {
		int symbolId = 1;
		for (int i = 0; i < xCount; ++i) {
			writeSymbol(symbolId);
			if (++symbolId == 5)
				symbolId = 1;
		}
	}

	_symbolPos.x = _bounds.width() - SYMBOL_WIDTH;
	writeSymbol(5);

	// Write the vertical edges
	if (yCount > 0) {
		int symbolId = 6;
		for (int i = 0; i < yCount; ++i) {
			_symbolPos.y += SYMBOL_HEIGHT;

			_symbolPos.x = 0;
			writeSymbol(symbolId);

			_symbolPos.x = _bounds.width() - SYMBOL_WIDTH;
			writeSymbol(symbolId + 4);

			if (++symbolId == 10)
				symbolId = 6;
		}
	}

	// Write the bottom line
	_symbolPos = Common::Point(0, _bounds.height() - SYMBOL_HEIGHT);
	writeSymbol(14);

	if (xCount > 0) {
		int symbolId = 15;
		for (int i = 0; i < xCount; ++i) {
			writeSymbol(symbolId);
			if (++symbolId == 19)
				symbolId = 15;
		}
	}

	_symbolPos.x = _bounds.width() - SYMBOL_WIDTH;
	writeSymbol(19);
}

void ScrollView::fill() {
	Graphics::ManagedSurface s = getSurface();
	s.fillRect(Common::Rect(FRAME_BORDER_SIZE, FRAME_BORDER_SIZE,
		s.w - FRAME_BORDER_SIZE, s.h - FRAME_BORDER_SIZE), 153);
}

void ScrollView::writeSymbol(int symbolId) {
	Graphics::ManagedSurface s = getSurface();
	const byte *srcP = &g_globals->SYMBOLS[symbolId][0];
	s.addDirtyRect(Common::Rect(_symbolPos.x, _symbolPos.y,
		_symbolPos.x + SYMBOL_WIDTH, _symbolPos.y + SYMBOL_HEIGHT));

	for (int yp = 0; yp < SYMBOL_HEIGHT; ++yp) {
		byte *destP = (byte *)s.getBasePtr(
			_symbolPos.x, _symbolPos.y + yp);

		for (int xp = 0; xp < SYMBOL_WIDTH; ++xp, ++destP) {
			byte b = *srcP++;
			if (b)
				*destP = b;
		}
	}

	_symbolPos.x += SYMBOL_WIDTH;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
