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

#include "ultima/ultima2/gfx/text_cursor.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

#define DELAY_FRAMES 5

constexpr int COLOR_CURSOR = 15; // white

TextCursor::TextCursor(const Common::String &name, UIElement *parent) : UIElement(name, parent) {
	setPosition(Common::Point(0, 0));
	delayFrames(DELAY_FRAMES);
}

void TextCursor::show() {
	_visible = true;
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
	auto s = getSurface();
	s.clear();

	if (_visible)
		s.fillRect(Common::Rect(0, 7, 8, 8), COLOR_CURSOR);
}

void TextCursor::timeout() {
	_visible = !_visible;
	redraw();
	delayFrames(DELAY_FRAMES);
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
