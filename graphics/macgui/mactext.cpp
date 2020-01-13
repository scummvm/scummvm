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

const Common::String MacFontRun::toString() {
	return Common::String::format("\001\016%04x%02x%04x%04x%04x%04x", fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);
}

MacText::~MacText() {
	delete _surface;
}

MacText::MacText(Common::U32String s, MacWindowManager *wm, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) {
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
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
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

MacText::MacText(const Common::String &s, MacWindowManager *wm, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) {
	_str = Common::U32String(s);
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
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
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

void MacText::setMaxWidth(int maxWidth) {
	_maxWidth = maxWidth;

	_textLines.clear();

	splitString(_str);

	recalcDims();

	_fullRefresh = true;
}

static const Common::U32String::value_type *readHex(uint16 *res, const Common::U32String::value_type *s, int len) {
	*res = 0;

	for (int i = 0; i < len; i++) {
		char b = (char)*s++;

		*res <<= 4;
		if (tolower(b) > 'a')
			*res |= tolower(b) - 'a' + 10;
		else
			*res |= tolower(b) - '0';
	}

	return s;
}

void MacText::splitString(Common::U32String &str) {
	const Common::U32String::value_type *s = str.c_str();

	Common::U32String tmp;
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
			debugN(9, "%2d ", i);

			for (uint j = 0; j < _textLines[i].chunks.size(); j++)
				debugN(9, "[%d] \"%s\"", _textLines[i].chunks[j].fontId, Common::toPrintable(_textLines[i].chunks[j].text.encode()).c_str());

			debug(9, " --> %c %d, '%s'", (*s > 0x20 ? *s : ' '), (byte)*s, Common::toPrintable(tmp.encode()).c_str());
		}
#endif

		if (*s == '\001') {
			s++;
			if (*s == '\001') {
				// Copy it verbatim
			} else if (*s == '\015') {
				s++;

				uint16 fontId = *s++; fontId = (fontId << 8) | *s++;
				byte textSlant = *s++;
				uint16 fontSize = *s++; fontSize = (fontSize << 8) | *s++;
				uint16 palinfo1 = *s++; palinfo1 = (palinfo1 << 8) | *s++;
				uint16 palinfo2 = *s++; palinfo2 = (palinfo2 << 8) | *s++;
				uint16 palinfo3 = *s++; palinfo3 = (palinfo3 << 8) | *s++;

				debug(9, "******** splitString: fontId: %d, textSlant: %d, fontSize: %d, p0: %x p1: %x p2: %x",
						fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				previousFormatting = _currentFormatting;
				_currentFormatting.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				if (curLine == 0 && curChunk == 0 && tmp.empty())
					previousFormatting = _currentFormatting;

				nextChunk = true;
			} else if (*s == '\016') {
				s++;

				uint16 fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3;

				s = readHex(&fontId, s, 4);
				s = readHex(&textSlant, s, 2);
				s = readHex(&fontSize, s, 4);
				s = readHex(&palinfo1, s, 4);
				s = readHex(&palinfo2, s, 4);
				s = readHex(&palinfo3, s, 4);

				debug(9, "******** splitString: fontId: %d, textSlant: %d, fontSize: %d, p0: %x p1: %x p2: %x",
						fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				previousFormatting = _currentFormatting;
				_currentFormatting.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				if (curLine == 0 && curChunk == 0 && tmp.empty())
					previousFormatting = _currentFormatting;

				nextChunk = true;
			} else {
				error("MacText: formatting error, got %02x", *s);
			}
		} else if (*s == '\n' && prevCR) {	// treat \r\n as one
			prevCR = false;

			s++;
			continue;
		} else if (*s == '\r') {
			prevCR = true;
		}

		if (*s == '\r' || *s == '\n' || nextChunk) {
			Common::Array<Common::U32String> text;

			if (!nextChunk)
				previousFormatting = _currentFormatting;

			int w = getLineWidth(curLine, true);

			previousFormatting.getFont()->wordWrapText(tmp, _maxWidth, text, w);
			tmp.clear();

			if (text.size()) {
				for (uint i = 0; i < text.size(); i++) {
					_textLines[curLine].chunks[curChunk].text += text[i];

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
		Common::Array<Common::U32String> text;
		int w = getLineWidth(curLine, true);

		_currentFormatting.getFont()->wordWrapText(tmp, _maxWidth, text, w);

		if (text.size())
			_textLines[curLine].chunks[curChunk].text = text[0];
		else
			warning("MacText::splitString(): Font resulted in 0 width for text '%s'", tmp.encode().c_str());

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
		n->clear(_bgcolor);
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
	if (_textLines.empty())
		return;

	reallocSurface();

	from = MAX<int>(0, from);
	to = MIN<int>(to, _textLines.size() - 1);

	int w = MIN(_maxWidth, _textMaxWidth);

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
			debug(9, "MacText::render: line %d[%d] h:%d at %d,%d (%s) fontid: %d on %dx%d",
				i, j, xOffset, _textLines[i].y, _textLines[i].height, _textLines[i].chunks[j].text.encode().c_str(),
				_textLines[i].chunks[j].fontId, _surface->w, _surface->h);

			if (_textLines[i].chunks[j].text.empty())
				continue;

			_textLines[i].chunks[j].getFont()->drawString(_surface, _textLines[i].chunks[j].text, xOffset, _textLines[i].y, w, _fgcolor);
			xOffset += _textLines[i].chunks[j].getFont()->getStringWidth(_textLines[i].chunks[j].text);
		}
	}

	for (uint i = 0; i < _textLines.size(); i++) {
		debugN(9, "MacText::render: %2d ", i);

		for (uint j = 0; j < _textLines[i].chunks.size(); j++)
			debugN(9, "[%d (%d)] \"%s\" ", _textLines[i].chunks[j].fontId, _textLines[i].chunks[j].textSlant, _textLines[i].chunks[j].text.encode().c_str());

		debug(9, "%s", "");
	}
}

int MacText::getLineWidth(int line, bool enforce) {
	if ((uint)line >= _textLines.size())
		return 0;

	if (_textLines[line].width != -1 && !enforce)
		return _textLines[line].width;

	int width = 0;
	int height = 0;

	bool hastext = false;

	for (uint i = 0; i < _textLines[line].chunks.size(); i++) {
		if (enforce)
			_textLines[line].chunks[i].font = nullptr;

		if (!_textLines[line].chunks[i].text.empty()) {
			width += _textLines[line].chunks[i].getFont()->getStringWidth(_textLines[line].chunks[i].text);
			hastext = true;
		}

		height = MAX(height, _textLines[line].chunks[i].getFont()->getFontHeight());
	}

	if (!hastext && _textLines.size() > 1)
		height = height > 3 ? height - 3 : 0;

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
	if (_textLines.empty())
		return;

	int y = 0;
	_textMaxWidth = 0;

	for (uint i = 0; i < _textLines.size(); i++) {
		_textLines[i].y = y;

		// We must calculate width first, because it enforces
		// the computation. Calling Height() will return cached value!
		_textMaxWidth = MAX(_textMaxWidth, getLineWidth(i, true));
		y += getLineHeight(i) + _interLinear;
	}

	_textMaxHeight = y - _interLinear;
}

void MacText::draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff) {
	if (_textLines.empty())
		return;

	render();

	if (x + w < _surface->w || y + h < _surface->h) {
		g->fillRect(Common::Rect(x, y, x + w, y + w), _bgcolor);
	}

	g->blitFrom(*_surface, Common::Rect(MIN<int>(_surface->w, x), MIN<int>(_surface->h, y),
										MIN<int>(_surface->w, x + w), MIN<int>(_surface->h, y + h)),
										Common::Point(xoff, yoff));
}

void MacText::drawToPoint(ManagedSurface *g, Common::Rect srcRect, Common::Point dstPoint) {
	if (_textLines.empty())
		return;

	render();

	srcRect.clip(_surface->getBounds());

	if (srcRect.isEmpty())
		return;

	g->blitFrom(*_surface, srcRect, dstPoint);
}

void MacText::drawToPoint(ManagedSurface *g, Common::Point dstPoint) {
	if (_textLines.empty())
		return;

	render();

	g->blitFrom(*_surface, dstPoint);
}

// Count newline characters in String
uint getNewlinesInString(const Common::U32String &str) {
	Common::U32String::const_iterator p = str.begin();
	uint newLines = 0;
	while (*p) {
		if (*p == '\n')
			newLines++;
		p++;
	}
	return newLines;
}

void MacText::appendText(Common::U32String str, int fontId, int fontSize, int fontSlant, bool skipAdd) {
	uint oldLen = _textLines.size();

	MacFontRun fontRun = MacFontRun(_wm, fontId, fontSlant, fontSize, 0, 0, 0);

	_currentFormatting = fontRun;

	if (!skipAdd) {
		_str += fontRun.toString();
		_str += str;
	}

	splitString(str);
	recalcDims();

	render(oldLen - 1, _textLines.size());
}

void MacText::appendText(const Common::String &str, int fontId, int fontSize, int fontSlant, bool skipAdd) {
	appendText(Common::U32String(str), fontId, fontSize, fontSlant, skipAdd);
}

void MacText::appendTextDefault(Common::U32String str, bool skipAdd) {
	uint oldLen = _textLines.size();

	_currentFormatting = _defaultFormatting;

	if (!skipAdd) {
		_str += _defaultFormatting.toString();
		_str += str;
	}

	splitString(str);
	recalcDims();

	render(oldLen - 1, _textLines.size());
}

void MacText::appendTextDefault(const Common::String &str, bool skipAdd) {
	appendTextDefault(Common::U32String(str), skipAdd);
}

void MacText::clearText() {
	_textLines.clear();
	_str.clear();

	if (_surface)
		_surface->clear(_bgcolor);

	recalcDims();
}

void MacText::replaceLastLine(Common::U32String str) {
	int oldLen = MAX<int>(0, _textLines.size() - 1);

	// TODO: Recalc length, adapt to _textLines

	if (_textLines.size())
		_textLines.pop_back();

	splitString(str);
	recalcDims();

	render(oldLen, _textLines.size());
}

void MacText::removeLastLine() {
	if (!_textLines.size())
		return;

	int h = getLineHeight(_textLines.size() - 1) + _interLinear;

	_surface->fillRect(Common::Rect(0, _textMaxHeight - h, _surface->w, _textMaxHeight), _bgcolor);

	_textLines.pop_back();
	_textMaxHeight -= h;
}

void MacText::getRowCol(int x, int y, int *sx, int *sy, int *row, int *col) {
	if (y > _textMaxHeight) {
		x = _surface->w;
	}

	y = CLIP(y, 0, _textMaxHeight);

	// FIXME: We should use bsearch() here
	*row = _textLines.size() - 1;

	while (*row && _textLines[*row].y > y)
		(*row)--;

	*sy = _textLines[*row].y;

	*col = 0;

	int width = 0, pwidth = 0;
	int mcol = 0, pmcol = 0;
	uint chunk;
	for (chunk = 0; chunk < _textLines[*row].chunks.size(); chunk++) {
		pwidth = width;
		pmcol = mcol;
		if (!_textLines[*row].chunks[chunk].text.empty()) {
			width += _textLines[*row].chunks[chunk].getFont()->getStringWidth(_textLines[*row].chunks[chunk].text);
			mcol += _textLines[*row].chunks[chunk].text.size();
		}

		if (width > x)
			break;
	}

	if (chunk == _textLines[*row].chunks.size())
		chunk--;

	Common::U32String str = _textLines[*row].chunks[chunk].text;

	*col = mcol;

	for (int i = str.size(); i >= 0; i--) {
		int strw = _textLines[*row].chunks[chunk].getFont()->getStringWidth(str);
		if (strw + pwidth < x) {
			*col = pmcol + i;
			*sx = strw + pwidth;
			break;
		}

		str.deleteLastChar();
	}
}

Common::U32String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::U32String res;

	startRow = CLIP(startRow, 0, (int)_textLines.size() - 1);
	endRow = CLIP(endRow, 0, (int)_textLines.size() - 1);

	for (int i = startRow; i <= endRow; i++) {
		if (i == startRow && i == endRow) {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (startCol <= 0) {
					if (formatted)
						res += _textLines[i].chunks[chunk].toString();

					if (endCol >= (int)_textLines[i].chunks[chunk].text.size())
						res += _textLines[i].chunks[chunk].text;
					else
						res += Common::U32String(_textLines[i].chunks[chunk].text.c_str(), endCol);
				} else if ((int)_textLines[i].chunks[chunk].text.size() > startCol) {
					if (formatted)
						res += _textLines[i].chunks[chunk].toString();

					res += Common::U32String(_textLines[i].chunks[chunk].text.c_str() + startCol, endCol - startCol);
				}

				startCol -= _textLines[i].chunks[chunk].text.size();
				endCol -= _textLines[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		} else if (i == startRow && startCol != 0) {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (startCol <= 0) {
					if (formatted)
						res += _textLines[i].chunks[chunk].toString();

					res += _textLines[i].chunks[chunk].text;
				} else if ((int)_textLines[i].chunks[chunk].text.size() > startCol) {
					if (formatted)
						res += _textLines[i].chunks[chunk].toString();

					res += Common::U32String(_textLines[i].chunks[chunk].text.c_str() + startCol);
				}

				startCol -= _textLines[i].chunks[chunk].text.size();
			}
			if (newlines)
				res += '\n';
			else
				res += ' ';
		} else if (i == endRow) {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (formatted)
					res += _textLines[i].chunks[chunk].toString();

				if (endCol >= (int)_textLines[i].chunks[chunk].text.size())
					res += _textLines[i].chunks[chunk].text;
				else
					res += Common::U32String(_textLines[i].chunks[chunk].text.c_str(), endCol);

				endCol -= _textLines[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		} else {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (formatted)
					res += _textLines[i].chunks[chunk].toString();

				res += _textLines[i].chunks[chunk].text;
			}

			if (newlines)
				res += '\n';
			else
				res += ' ';
		}
	}

	return res;
}

} // End of namespace Graphics
