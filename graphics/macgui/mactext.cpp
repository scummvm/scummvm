/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "graphics/macgui/mactext.h"
#include "graphics/font.h"

namespace Graphics {

MacText::MacText(Common::String s, Graphics::Font *font, int fgcolor, int bgcolor, int maxWidth) {
	_str = s;
	_font = font;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_maxWidth = maxWidth;
	_surface = nullptr;

	_interLinear = 0; // 0 pixels between the lines by default

	if (_maxWidth == -1)
		_textMaxWidth = 1000000; // Some big value
	else
		_textMaxWidth = -1;

	splitString(_str);

	_fullRefresh = true;
}

void MacText::splitString(Common::String &str) {
	const char *s = str.c_str();

	Common::String tmp;
	bool prevCR = false;

	while (*s) {
		if (*s == '\n' && prevCR) {	// trean \r\n as one
			prevCR = false;
			continue;
		}

		if (*s == '\r')
			prevCR = true;

		if (*s == '\r' || *s == '\n') {
			_maxWidth = MIN(_font->wordWrapText(tmp, _maxWidth, _text), _maxWidth);

			tmp.clear();

			continue;
		}

		tmp += *s;
	}

	if (tmp.size())
		_maxWidth = MIN(_font->wordWrapText(tmp, _maxWidth, _text), _maxWidth);
}

void MacText::reallocSurface() {
	int lineH = _font->getFontHeight() + _interLinear;
	// round to closest 10
	int requiredH = (_text.size() + (_text.size() * 10 + 9) / 10) * lineH;
	int surfW = _maxWidth == -1 ? _textMaxWidth : _maxWidth;

	if (!_surface) {
		_surface = new ManagedSurface(surfW, requiredH);

		return;
	}

	if (_surface->h < requiredH) {
		// realloc surface and copy old content
		ManagedSurface *n = new ManagedSurface(surfW, requiredH);
		n->blitFrom(*_surface, Common::Point(0, 0));

		delete _surface;
		_surface = n;
	}
}

void MacText::render() {
	if (_fullRefresh) {
		render(0, _text.size());

		_fullRefresh = false;
	}
}

void MacText::render(int from, int to) {
	reallocSurface();

	from = MAX<int>(0, from);
	to = MIN<int>(to, _text.size());

	int lineH = _font->getFontHeight() + _interLinear;
	int y = from * lineH;

	// Clear the screen
	_surface->fillRect(Common::Rect(0, y, _surface->w, to * lineH), _bgcolor);

	for (int i = from; i < to; i++) {
		_font->drawString(_surface, _text[i], 0, y, _textMaxWidth, _fgcolor);

		y += _font->getFontHeight() + _interLinear;
	}

}

void MacText::draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff) {
	render();

	if (x + w < _surface->w || y + h < _surface->h) {
		g->fillRect(Common::Rect(x, y, x + w, y + w), _bgcolor);
	}

	g->blitFrom(*_surface, Common::Rect(MIN<int>(_surface->w, x),     MIN<int>(_surface->h, y),
									    MIN<int>(_surface->w, x + w), MIN<int>(_surface->w, y + w)),
										Common::Point(xoff, yoff));
}

void MacText::appendText(Common::String str) {
	int oldLen = _text.size();

	splitString(str);

	render(oldLen + 1, _text.size());
}

void MacText::replaceLastLine(Common::String str) {
	int oldLen = MAX<int>(0, _text.size() - 1);

	if (_text.size())
		_text.pop_back();

	splitString(str);

	render(oldLen, _text.size());
}

} // End of namespace Graphics
