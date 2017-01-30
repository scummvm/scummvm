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

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/font.h"

namespace Graphics {

const Font *MacFontRun::getFont() {
	if (font)
		return font;

	MacFont macFont = MacFont(fontId, fontSize, textSlant);

	font = wm->_fontMan->getFont(macFont);

	return font;
}

MacText::MacText(Common::String s, MacWindowManager *wm, const Font *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment) {
	_str = s;
	_wm = wm;
	_font = font;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_maxWidth = maxWidth;
	_textMaxWidth = 0;
	_surface = nullptr;
	_textAlignment = textAlignment;

	_interLinear = 0; // 0 pixels between the lines by default

	splitString(_str);

	_fullRefresh = true;

	_defaultFormatting.font = font;
	_defaultFormatting.wm = wm;

	_currentFormatting = _defaultFormatting;
}

void MacText::splitString(Common::String &str) {
	const char *s = str.c_str();

	Common::String tmp;
	bool prevCR = false;

	int curLine = _text.empty() ? 0 : _text.size() - 1;

	while (*s) {
		if (*s == '\001') {
			s++;
			if (*s == '\001') {
				// Copy it verbatim
			} else {
				if (*s++ != '\015')
					error("MacText: formatting error");

				uint16 fontId = *s++; fontId = (fontId << 8) | *s++;
				byte textSlant = *s++;
				byte unk3f = *s++;
				uint16 fontSize = *s++; fontSize = (fontSize << 8) | *s++;
				uint16 palinfo1 = *s++; palinfo1 = (palinfo1 << 8) | *s++;
				uint16 palinfo2 = *s++; palinfo2 = (palinfo2 << 8) | *s++;
				uint16 palinfo3 = *s++; palinfo3 = (palinfo3 << 8) | *s++;

				debug(8, "******** splitString: fontId: %d, textSlant: %d, unk3: %d, fontSize: %d, p0: %x p1: %x p2: %x",
						fontId, textSlant, unk3f, fontSize, palinfo1, palinfo2, palinfo3);

				_currentFormatting.setValues(_wm, fontId, textSlant, unk3f, fontSize, palinfo1, palinfo2, palinfo3);

				if (_formatting.empty())
					_formatting.resize(1);

				_formatting[curLine].push_back(_currentFormatting);

				continue;
			}
		}

		if (*s == '\n' && prevCR) {	// trean \r\n as one
			prevCR = false;

			s++;
			continue;
		}

		if (*s == '\r')
			prevCR = true;

		if (*s == '\r' || *s == '\n') {
			_textMaxWidth = MAX(_font->wordWrapText(tmp, _maxWidth, _text), _textMaxWidth);

			tmp.clear();

			s++;
			continue;
		}

		tmp += *s;
		s++;
	}

	if (tmp.size())
		_textMaxWidth = MAX(_font->wordWrapText(tmp, _maxWidth, _text), _textMaxWidth);
}

void MacText::reallocSurface() {
	int lineH = _font->getFontHeight() + _interLinear;
	// round to closest 10
	//TODO: work out why this rounding doesn't correctly fill the entire width
	//int requiredH = (_text.size() + (_text.size() * 10 + 9) / 10) * lineH
	int requiredH = _text.size() * lineH;
	int surfW = _textMaxWidth;

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
		int xOffset = 0;
		if (_textAlignment == kTextAlignRight)
			xOffset = _textMaxWidth - _font->getStringWidth(_text[i]);
		else if (_textAlignment == kTextAlignCenter)
			xOffset = (_textMaxWidth / 2) - (_font->getStringWidth(_text[i]) / 2);

		//TODO: _textMaxWidth, when -1, was not rendering ANY text.
		_font->drawString(_surface, _text[i], xOffset, y, _maxWidth, _fgcolor);

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
