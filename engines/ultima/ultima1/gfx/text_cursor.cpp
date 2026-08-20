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

#include "ultima/ultima1/gfx/text_cursor.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

#define DELAY_FRAMES 2
constexpr int COLOR_CURSOR = 11; // light cyan

// 4-frame 8x8 glyph, one byte per row, low bit = leftmost pixel
static const byte CURSOR_FRAMES[4][8] = {
	{ 0x66, 0x3C, 0x18, 0x66, 0x66, 0x18, 0x3C, 0x66 },
	{ 0x3C, 0x18, 0x66, 0x24, 0x24, 0x66, 0x18, 0x3C },
	{ 0x18, 0x66, 0x24, 0x3C, 0x3C, 0x24, 0x66, 0x18 },
	{ 0x66, 0x24, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x66 }
};

TextCursor::TextCursor(const Common::String &name, UIElement *parent) : UIElement(name, parent) {
	delayFrames(DELAY_FRAMES);
}

void TextCursor::show() {
	_visible = true;
	_frame = 0;
	redraw();
	delayFrames(DELAY_FRAMES);
}

void TextCursor::hide() {
	_visible = false;
	cancelDelay();
	redraw();
}

void TextCursor::setPosition(const Common::Point &pt) {
	setBounds(TextRect(pt.x, pt.y, pt.x, pt.y));
}

void TextCursor::draw() {
	if (!_visible)
		return;

	auto s = getSurface();
	s.clear();

	for (int y = 0; y < 8; ++y) {
		byte bits = CURSOR_FRAMES[_frame][y];
		for (int x = 0; x < 8; ++x, bits >>= 1) {
			if (bits & 1)
				s.drawLine(x, y, x, y, COLOR_CURSOR);
		}
	}
}

void TextCursor::timeout() {
	if (_visible) {
		_frame = (_frame + 1) % 4;
		redraw();
		delayFrames(DELAY_FRAMES);
	}
}

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima
