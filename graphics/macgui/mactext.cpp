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


MacText::~MacText(){
	delete _macFont;
}

MacText::MacText(Common::String s, MacWindowManager *wm, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) {
	_str = s;
	_wm = wm;
	_macFont = macFont;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_maxWidth = maxWidth;
	_textMaxWidth = 0;
	_textMaxHeight = 0;
	_surface = nullptr;
	_textAlignment = textAlignment;
	_interLinear = interlinear;

	if (macFont) {
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
	} else {
		_defaultFormatting.font = NULL;
	}

	_defaultFormatting.wm = wm;

	_currentFormatting = _defaultFormatting;

	splitString(_str);

	recalcDims();

	_fullRefresh = true;
}

void MacText::splitString(Common::String &str) {
	const char *s = str.c_str();

	Common::String tmp;
	bool prevCR = false;

	if (_textLines.empty()) {
		_textLines.resize(1);
		_textLines[0].chunks.push_back(_defaultFormatting);
	}

	int curLine = _textLines.size() - 1;
	int curChunk = _textLines[curLine].chunks.size() - 1;
	bool nextChunk = false;
	MacFontRun previousFormatting;

	while (*s) {
#if DEBUG
		for (uint i = 0; i < _textLines.size(); i++) {
			debugN(7, "%2d ", i);

			for (uint j = 0; j < _textLines[i].chunks.size(); j++)
				debugN(7, "[%d] \"%s\"", _textLines[i].chunks[j].fontId, _textLines[i].chunks[j].text.c_str());

			debug(7, " --> %c %d, '%s'", (*s > 0x20 ? *s : ' '), (byte)*s, tmp.c_str());
		}
#endif

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

				previousFormatting = _currentFormatting;
				_currentFormatting.setValues(_wm, fontId, textSlant, unk3f, fontSize, palinfo1, palinfo2, palinfo3);

				if (curLine == 0 && curChunk == 0 && tmp.empty())
					previousFormatting = _currentFormatting;

				nextChunk = true;
			}
		} else if (*s == '\n' && prevCR) {	// trean \r\n as one
			prevCR = false;

			s++;
			continue;
		} else if (*s == '\r') {
			prevCR = true;
		}

		if (*s == '\r' || *s == '\n' || nextChunk) {
			Common::Array<Common::String> text;

			if (!nextChunk)
				previousFormatting = _currentFormatting;

			int w = getLineWidth(curLine, true);

			previousFormatting.getFont()->wordWrapText(tmp, _maxWidth, text, w);
			tmp.clear();

			if (text.size()) {
				for (uint i = 0; i < text.size(); i++) {
					_textLines[curLine].chunks[curChunk].text = text[i];

					if ((text.size() > 1 || !nextChunk) && !(i == text.size() - 1 && nextChunk)) {
						curLine++;
						_textLines.resize(curLine + 1);
						_textLines[curLine].chunks.push_back(previousFormatting);
						curChunk = 0;
					}
				}

				if (nextChunk) {
					curChunk++;

					_textLines[curLine].chunks.push_back(_currentFormatting);
				} else {
					_textLines[curLine].chunks[0] = _currentFormatting;
				}
			} else {
				if (nextChunk) { // No text, replacing formatting
					_textLines[curLine].chunks[curChunk] = _currentFormatting;
				} else { // Otherwise it is an empty line
					curLine++;
					_textLines.resize(curLine + 1);
					_textLines[curLine].chunks.push_back(previousFormatting);
					curChunk = 0;
				}
			}

			if (!nextChunk) // Don't skip next character
				s++;

			nextChunk = false;
			continue;
		}

		tmp += *s;
		s++;
	}

	if (tmp.size()) {
		Common::Array<Common::String> text;
		int w = getLineWidth(curLine, true);

		_currentFormatting.getFont()->wordWrapText(tmp, _maxWidth, text, w);

		_textLines[curLine].chunks[curChunk].text = text[0];

		if (text.size() > 1) {
			for (uint i = 1; i < text.size(); i++) {
				curLine++;
				_textLines.resize(curLine + 1);
				_textLines[curLine].chunks.push_back(_currentFormatting);
				_textLines[curLine].chunks[0].text = text[i];
			}
		}
	}
}

void MacText::reallocSurface() {
	// round to closest 10
	//TODO: work out why this rounding doesn't correctly fill the entire width
	//int requiredH = (_text.size() + (_text.size() * 10 + 9) / 10) * lineH

	if (!_surface) {
		_surface = new ManagedSurface(_textMaxWidth, _textMaxHeight);

		return;
	}

	if (_surface->w < _textMaxWidth || _surface->h < _textMaxHeight) {
		// realloc surface and copy old content
		ManagedSurface *n = new ManagedSurface(_textMaxWidth, _textMaxHeight);
		n->blitFrom(*_surface, Common::Point(0, 0));

		delete _surface;
		_surface = n;
	}
}

void MacText::render() {
	if (_fullRefresh) {
		render(0, _textLines.size());

		_fullRefresh = false;
	}
}

void MacText::render(int from, int to) {
	reallocSurface();

	from = MAX<int>(0, from);
	to = MIN<int>(to, _textLines.size() - 1);

	// Clear the screen
	_surface->fillRect(Common::Rect(0, _textLines[from].y, _surface->w, _textLines[to].y + getLineHeight(to)), _bgcolor);

	for (int i = from; i <= to; i++) {
		int xOffset = 0;
		if (_textAlignment == kTextAlignRight)
			xOffset = _textMaxWidth - getLineWidth(i);
		else if (_textAlignment == kTextAlignCenter)
			xOffset = (_textMaxWidth / 2) - (getLineWidth(i) / 2);

		// TODO: _textMaxWidth, when -1, was not rendering ANY text.
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			if (_textLines[i].chunks[j].text.empty())
				continue;

			_textLines[i].chunks[j].getFont()->drawString(_surface, _textLines[i].chunks[j].text, xOffset, _textLines[i].y, _maxWidth, _fgcolor);
			xOffset += _textLines[i].chunks[j].getFont()->getStringWidth(_textLines[i].chunks[j].text);
		}
	}

	for (uint i = 0; i < _textLines.size(); i++) {
		debugN(4, "%2d ", i);

		for (uint j = 0; j < _textLines[i].chunks.size(); j++)
			debugN(4, "[%d (%d)] \"%s\" ", _textLines[i].chunks[j].fontId, _textLines[i].chunks[j].textSlant, _textLines[i].chunks[j].text.c_str());

		debug(4, "%s", "");
	}
}

int MacText::getLineWidth(int line, bool enforce) {
	if ((uint)line >= _textLines.size())
		return 0;

	if (_textLines[line].width != -1 && !enforce)
		return _textLines[line].width;

	int width = 0;
	int height = 0;

	for (uint i = 0; i < _textLines[line].chunks.size(); i++) {
		if (enforce)
			_textLines[line].chunks[i].font = nullptr;

		if (!_textLines[line].chunks[i].text.empty())
			width += _textLines[line].chunks[i].getFont()->getStringWidth(_textLines[line].chunks[i].text);

		height = MAX(height, _textLines[line].chunks[i].getFont()->getFontHeight());
	}

	_textLines[line].width = width;
	_textLines[line].height = height;

	return width;
}

int MacText::getLineHeight(int line) {
	if ((uint)line >= _textLines.size())
		return 0;

	getLineWidth(line); // This calculates height also

	return _textLines[line].height;
}

void MacText::setInterLinear(int interLinear) { 
	_interLinear = interLinear; 
	recalcDims();
}

void MacText::recalcDims() {
	int y = 0;
	_textMaxWidth = 0;

	for (uint i = 0; i < _textLines.size(); i++) {
		_textLines[i].y = y;

		y += getLineHeight(i) + _interLinear;
		_textMaxWidth = MAX(_textMaxWidth, getLineWidth(i, true));
	}

	_textMaxHeight = y - _interLinear;
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
	int oldLen = _textLines.size();

	// TODO: Recalc length

	splitString(str);
	recalcDims();

	render(oldLen + 1, _textLines.size());
}

void MacText::replaceLastLine(Common::String str) {
	int oldLen = MAX<int>(0, _textLines.size() - 1);

	// TODO: Recalc length, adapt to _textLines

	if (_textLines.size())
		_textLines.pop_back();

	splitString(str);
	recalcDims();

	render(oldLen, _textLines.size());
}

} // End of namespace Graphics
