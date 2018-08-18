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
	return Common::String::format("\001\015%c%c%c%c%c%c%c%c%c%c%c",
			(fontId >> 8) & 0xff, fontId & 0xff,
			textSlant & 0xff,
			(fontSize >> 8) & 0xff, fontSize & 0xff,
			(palinfo1 >> 8) & 0xff, palinfo1 & 0xff,
			(palinfo2 >> 8) & 0xff, palinfo2 & 0xff,
			(palinfo3 >> 8) & 0xff, palinfo3 & 0xff);
}

MacText::~MacText() {
	delete _surface;
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

	if (!_str.empty())
		splitString(_str);

	recalcDims();

	_fullRefresh = true;
}

void MacText::setMaxWidth(int maxWidth) {
	_maxWidth = maxWidth;

	_textLines.clear();

	if (!_str.empty()) {
		splitString(_str);

		recalcDims();

		_fullRefresh = true;
	}
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
				uint16 fontSize = *s++; fontSize = (fontSize << 8) | *s++;
				uint16 palinfo1 = *s++; palinfo1 = (palinfo1 << 8) | *s++;
				uint16 palinfo2 = *s++; palinfo2 = (palinfo2 << 8) | *s++;
				uint16 palinfo3 = *s++; palinfo3 = (palinfo3 << 8) | *s++;

				debug(8, "******** splitString: fontId: %d, textSlant: %d, fontSize: %d, p0: %x p1: %x p2: %x",
						fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				previousFormatting = _currentFormatting;
				_currentFormatting.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

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
			debug(5, "line %d[%d]/%d at %d,%d (%s)", i, j, xOffset, _textLines[i].chunks[j].fontId, _textLines[i].y, _textLines[i].chunks[j].text.c_str());

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

// Count newline characters in String
uint getNewlinesInString(const Common::String &str) {
	Common::String::const_iterator p = str.begin();
	uint newLines = 0;
	while (*p) {
		if (*p == '\n')
			newLines++;
		p++;
	}
	return newLines;
}

void MacText::appendText(Common::String str, int fontId, int fontSize, int fontSlant, bool skipAdd) {
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

void MacText::appendTextDefault(Common::String str, bool skipAdd) {
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

void MacText::clearText() {
	_textLines.clear();
	_str.clear();

	if (_surface)
		_surface->clear(_bgcolor);

	recalcDims();
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

	Common::String str = _textLines[*row].chunks[chunk].text;

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

Common::String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::String res;

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
						res += Common::String(_textLines[i].chunks[chunk].text.c_str(), endCol);
				} else if ((int)_textLines[i].chunks[chunk].text.size() > startCol) {
					if (formatted)
						res += _textLines[i].chunks[chunk].toString();

					res += Common::String(_textLines[i].chunks[chunk].text.c_str() + startCol, endCol - startCol);
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

					res += Common::String(_textLines[i].chunks[chunk].text.c_str() + startCol);
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
					res += Common::String(_textLines[i].chunks[chunk].text.c_str(), endCol);

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
