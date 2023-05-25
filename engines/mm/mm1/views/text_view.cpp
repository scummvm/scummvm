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

#include "mm/mm1/globals.h"
#include "mm/mm1/gfx/gfx.h"
#include "mm/mm1/views/text_view.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Views {

TextView::TextView(const Common::String &name) :
		UIElement(name, g_engine) {
}

TextView::TextView(const Common::String &name, UIElement *owner) :
		UIElement(name, owner) {
}

void TextView::writeChar(unsigned char c) {
	if (c == '\r' || c == '\n') {
		_textPos.x = 0;
		_textPos.y++;
	} else {
		Graphics::ManagedSurface s = getSurface();
		g_globals->_font.drawChar(&s, c,
			_textPos.x * FONT_SIZE, _textPos.y * FONT_SIZE, _bgColor);

		if (++_textPos.x == TEXT_W) {
			_textPos.x = 0;
			++_textPos.y;
		}
	}
}

void TextView::writeChar(int x, int y, unsigned char c) {
	_textPos.x = x;
	_textPos.y = y;
	writeChar(c);
}

void TextView::writeString(const Common::String &str) {
	for (const unsigned char *s = (const unsigned char *)str.c_str(); *s; ++s) {
		writeChar(*s);

		if (*s >= ' ' && _textPos.x == 0 && (*(s + 1) == '\r' || *(s + 1) == '\n'))
			// Ignore carraige returns right after line wraps. The original didn't
			// have them and just carried on immedaitely with the next word,
			// but I wanted them to be present to cleanly identify line breaks
			++s;
	}
}

void TextView::writeString(int x, int y, const Common::String &str) {
	_textPos.x = x;
	_textPos.y = y;
	writeString(str);
}

void TextView::writeNumber(int val) {
	writeString(Common::String::format("%d", val));
}

void TextView::writeNumber(int x, int y, int val) {
	_textPos.x = x;
	_textPos.y = y;
	writeNumber(val);
}

void TextView::newLine() {
	_textPos.x = 0;
	_textPos.y++;
}

void TextView::writeSpaces(size_t count) {
	for (size_t i = 0; i < count; ++i)
		writeChar(' ');
}

void TextView::clearSurface() {
	UIElement::clearSurface();
	_textPos.x = _textPos.y = 0;
}

void TextView::clearLines(int y1, int y2) {
	Graphics::ManagedSurface s = getSurface();
	s.fillRect(getLineBounds(y1, y2), 0);
}

void TextView::drawTextBorder() {
	Graphics::ManagedSurface surf = getSurface();
	clearSurface();

	// Draw boxes in the four corners
	surf.fillRect(Common::Rect(0, 0, FONT_SIZE * 3, FONT_SIZE * 3), 255);
	surf.fillRect(Common::Rect(SCREEN_W - FONT_SIZE * 3, 0,
		SCREEN_W, FONT_SIZE * 3), 255);
	surf.fillRect(Common::Rect(0, SCREEN_H - FONT_SIZE * 3,
		FONT_SIZE * 3, SCREEN_H), 255);
	surf.fillRect(Common::Rect(SCREEN_W - FONT_SIZE * 3,
		SCREEN_H - FONT_SIZE * 3, SCREEN_W, SCREEN_H), 255);

	// Draw horizontal vertical lines
	for (int x = 3; x < TEXT_W - 3; ++x) {
		writeChar(x, 1, '-');
		writeChar(x, TEXT_H - 2, '-');
	}
	for (int y = 3; y < TEXT_H - 3; ++y) {
		writeChar(1, y, '!');
		writeChar(TEXT_W - 2, y, '!');
	}
}

void TextView::escToGoBack(int x, int y) {
	if (y == -1)
		y = (_bounds.height() / FONT_SIZE) - 1;

	writeString(x, y, STRING["dialogs.misc.go_back"]);
}

void TextView::timeout() {
	redraw();
}

bool TextView::msgDrawGraphic(const DrawGraphicMessage &msg) {
	// Refresh the background
	draw();

	// Draw the new image
	drawGraphic(msg._gfxNum);
	return true;
}

void TextView::drawGraphic(int gfxNum) {
	const Graphics::ManagedSurface img =
		g_globals->_monsters->getMonsterImage(gfxNum);
	getSurface().blitFrom(img, Common::Point(64, 16));
}

} // namespace Views
} // namespace MM1
} // namespace MM
