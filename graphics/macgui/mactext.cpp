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

#define DEBUG 0

#if DEBUG
#define D(...)  debug(__VA_ARGS__)
#else
#define D(...)  ;
#endif

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

bool MacFontRun::equals(MacFontRun &to) {
	return (fontId == to.fontId && textSlant == to.textSlant
		&& fontSize == to.fontSize && palinfo1 == to.palinfo1
		&& palinfo2 == to.palinfo2 && palinfo3 == to.palinfo3);
}

uint MacTextLine::getChunkNum(int *col) {
	int pos = *col;
	uint i;

	for (i = 0; i < chunks.size(); i++) {
		if (pos >= chunks[i].text.size()) {
			pos -= chunks[i].text.size();
		} else {
			break;
		}
	}

	if (i == chunks.size()) {
		i--;	// touch the last chunk
		pos = chunks[i].text.size() - 1;
	}

	*col = pos;

	return i;
}

MacText::~MacText() {
	delete _surface;
}

MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) {
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
		_defaultFormatting.font = nullptr;
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

void MacText::setDefaultFormatting(uint16 fontId, byte textSlant, uint16 fontSize,
		uint16 palinfo1, uint16 palinfo2, uint16 palinfo3) {
	_defaultFormatting.setValues(_defaultFormatting.wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

	MacFont macFont = MacFont(fontId, fontSize, textSlant);

	_defaultFormatting.font = _wm->_fontMan->getFont(macFont);
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

// Adds the given string to the end of the last line/chunk
// while observing the _maxWidth and keeping this chunk's
// formatting
void MacText::chopChunk(const Common::U32String &str) {
	int curLine = _textLines.size() - 1;
	int curChunk = _textLines[curLine].chunks.size() - 1;
	MacFontRun *chunk = &_textLines[curLine].chunks[curChunk];

	// Check if there is nothing to add, then remove the last chunk
	// This happens when the previous run is finished only with
	// empty formatting, or when we were adding text for the first time
	if (chunk->text.empty() && str.empty()) {
		D(9, "** chopChunk, replaced formatting, line %d", curLine);

		_textLines[curLine].chunks.pop_back();

		return;
	}

	Common::Array<Common::U32String> text;

	int w = getLineWidth(curLine, true);

	chunk->getFont()->wordWrapText(str, _maxWidth, text, w);

	if (text.size() == 0) {
		warning("chopChunk: too narrow width, >%d", _maxWidth);
		chunk->text += str;
		getLineCharWidth(curLine, true);

		return;
	}

	chunk->text += text[0];

	// Recalc dims
	getLineWidth(curLine, true);

	D(9, "** chopChunk, subchunk: \"%s\" (%d lines, maxW: %d)", toPrintable(text[0].encode()).c_str(), text.size(), _maxWidth);

	// We do not overlap, so we're done
	if (text.size() == 1)
		return;

	// Now add rest of the chunks
	MacFontRun newchunk = _textLines[curLine].chunks[curChunk];

	for (uint i = 1; i < text.size(); i++) {
		newchunk.text = text[i];

		curLine++;
		_textLines.resize(curLine + 1);
		_textLines[curLine].chunks.push_back(newchunk);

		D(9, "** chopChunk, added line: \"%s\"", toPrintable(text[i].encode()).c_str());
	}
}

void MacText::splitString(const Common::U32String &str) {
	const Common::U32String::value_type *l = str.c_str();

	D(9, "** splitString(\"%s\")", toPrintable(str.encode()).c_str());

	if (str.empty()) {
		debug(9, "** splitString, empty line");
		return;
	}

	Common::U32String paragraph, tmp;

	if (_textLines.empty()) {
		_textLines.resize(1);
		_textLines[0].chunks.push_back(_defaultFormatting);
		D(9, "** splitString, added default formatting");
	} else {
		D(9, "** splitString, continuing, %d lines", _textLines.size());
	}

	int curLine = _textLines.size() - 1;
	int curChunk = _textLines[curLine].chunks.size() - 1;
	MacFontRun chunk = _textLines[curLine].chunks[curChunk];

	while (*l) {
		paragraph.clear();

		// First, get the whole paragraph
		while (*l) {
			if (*l == '\r') {
				l++;

				if (*l == '\n')	// Skip whole '\r\n'
					l++;

				break;
			}

			paragraph += *l++;
		}

		D(9, "** splitString, paragraph: \"%s\"", Common::toPrintable(line.encode()).c_str());

		// Now process whole paragraph
		const Common::U32String::value_type *s = paragraph.c_str();

		tmp.clear();

		while (*s) {
			// Scan till next font change or end of line
			while (*s && *s != '\001') {
				tmp += *s;

				s++;
			}

			if (*s)	// If it was \001, skip it
				s++;

			if (*s == '\001') { // \001\001 -> \001
				tmp += *s++;

				if (*s)	// Check we reached end of line
					continue;
			}

			D(9, "** splitString, chunk: \"%s\"", Common::toPrintable(tmp.encode()).c_str());

			// Okay, now we are either at the end of the line, or in the next
			// chunk definition. That means, that we have to store the previous chunk
			chopChunk(tmp);

			tmp.clear();

			// If it is end of the line, we're done
			if (!*s) {
				D(9, "** splitString, end of line");

				break;
			}

			if (*s == '\015') {	// binary format
				s++;

				uint16 fontId = *s++; fontId = (fontId << 8) | *s++;
				byte textSlant = *s++;
				uint16 fontSize = *s++; fontSize = (fontSize << 8) | *s++;
				uint16 palinfo1 = *s++; palinfo1 = (palinfo1 << 8) | *s++;
				uint16 palinfo2 = *s++; palinfo2 = (palinfo2 << 8) | *s++;
				uint16 palinfo3 = *s++; palinfo3 = (palinfo3 << 8) | *s++;

				D(9, "** splitString: fontId: %d, textSlant: %d, fontSize: %d, p0: %x p1: %x p2: %x",
						fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				chunk.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);
			} else if (*s == '\016') {	// human-readable format
				s++;

				uint16 fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3;

				s = readHex(&fontId, s, 4);
				s = readHex(&textSlant, s, 2);
				s = readHex(&fontSize, s, 4);
				s = readHex(&palinfo1, s, 4);
				s = readHex(&palinfo2, s, 4);
				s = readHex(&palinfo3, s, 4);

				D(9, "** splitString: fontId: %d, textSlant: %d, fontSize: %d, p0: %x p1: %x p2: %x",
						fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				chunk.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);
			} else {
				error("MacText: formatting error, got %02x", *s);
			}

			// Push new formatting
			_textLines[curLine].chunks.push_back(chunk);
		}

		if (!*l) { // If this is end of the string, we're done here
			break;
		}

		// Add new line
		D(9, "** splitString: new line");

		_textLines[curLine].paragraphEnd = true;

		curLine++;
		_textLines.resize(curLine + 1);
		_textLines[curLine].chunks.push_back(chunk);
	}

#if DEBUG
	for (uint i = 0; i < _textLines.size(); i++) {
		debugN(9, "** splitString: %2d ", i);

		for (uint j = 0; j < _textLines[i].chunks.size(); j++)
			debugN(9, "[%d] \"%s\"", _textLines[i].chunks[j].fontId, Common::toPrintable(_textLines[i].chunks[j].text.encode()).c_str());

		debugN(9, "\n");
	}
	debug(9, "** splitString: done");
#endif
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

		int maxHeightForRow = 0;
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			if (_textLines[i].chunks[j].font->getFontHeight() > maxHeightForRow)
				maxHeightForRow = _textLines[i].chunks[j].font->getFontHeight();
		}

		// TODO: _textMaxWidth, when -1, was not rendering ANY text.
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			debug(9, "MacText::render: line %d[%d] h:%d at %d,%d (%s) fontid: %d on %dx%d",
				i, j, xOffset, _textLines[i].y, _textLines[i].height, _textLines[i].chunks[j].text.encode().c_str(),
				_textLines[i].chunks[j].fontId, _surface->w, _surface->h);

			if (_textLines[i].chunks[j].text.empty())
				continue;

			//TODO: There might be a vertical alignment setting somewhere for differing font sizes in a single row?
			int yOffset = 0;
			if (_textLines[i].chunks[j].font->getFontHeight() < maxHeightForRow) {
				//TODO: determine where the magic value 2 comes from
				yOffset = maxHeightForRow - _textLines[i].chunks[j].font->getFontHeight() - 2;
			}

			_textLines[i].chunks[j].getFont()->drawString(_surface, _textLines[i].chunks[j].text, xOffset, _textLines[i].y + yOffset, w, _fgcolor);
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

int MacText::getLineWidth(int line, bool enforce, int col) {
	if ((uint)line >= _textLines.size())
		return 0;

	if (_textLines[line].width != -1 && !enforce && col == -1)
		return _textLines[line].width;

	int width = 0;
	int height = 0;
	int charwidth = 0;

	bool hastext = false;

	for (uint i = 0; i < _textLines[line].chunks.size(); i++) {
		if (enforce)
			_textLines[line].chunks[i].font = nullptr;

		if (col >= 0) {
			if (col >= _textLines[line].chunks[i].text.size()) {
				col -= _textLines[line].chunks[i].text.size();
			} else {
				Common::U32String tmp(_textLines[line].chunks[i].text.c_str(), col);

				width += _textLines[line].chunks[i].getFont()->getStringWidth(tmp);

				return width;
			}
		}

		if (!_textLines[line].chunks[i].text.empty()) {
			width += _textLines[line].chunks[i].getFont()->getStringWidth(_textLines[line].chunks[i].text);
			charwidth += _textLines[line].chunks[i].text.size();
			hastext = true;
		}

		height = MAX(height, _textLines[line].chunks[i].getFont()->getFontHeight());
	}

	if (!hastext && _textLines.size() > 1)
		height = height > 3 ? height - 3 : 0;

	_textLines[line].width = width;
	_textLines[line].height = height;
	_textLines[line].charwidth = charwidth;

	return width;
}

int MacText::getLineCharWidth(int line, bool enforce) {
	if ((uint)line >= _textLines.size())
		return 0;

	if (_textLines[line].charwidth != -1 && !enforce)
		return _textLines[line].charwidth;

	int width = 0;

	for (uint i = 0; i < _textLines[line].chunks.size(); i++) {
		if (!_textLines[line].chunks[i].text.empty())
			width += _textLines[line].chunks[i].text.size();
	}

	_textLines[line].charwidth = width;

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

void MacText::appendText(const Common::U32String &str, int fontId, int fontSize, int fontSlant, bool skipAdd) {
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

void MacText::appendTextDefault(const Common::U32String &str, bool skipAdd) {
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

void MacText::replaceLastLine(const Common::U32String &str) {
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
	int nsx, nsy, nrow, ncol;

	if (y > _textMaxHeight) {
		x = _surface->w;
	}

	y = CLIP(y, 0, _textMaxHeight);

	// FIXME: We should use bsearch() here
	nrow = _textLines.size() - 1;

	while (nrow && _textLines[nrow].y > y)
		(nrow)--;

	nsy = _textLines[nrow].y;

	ncol = 0;

	int width = 0, pwidth = 0;
	int mcol = 0, pmcol = 0;
	uint chunk;
	for (chunk = 0; chunk < _textLines[nrow].chunks.size(); chunk++) {
		pwidth = width;
		pmcol = mcol;
		if (!_textLines[nrow].chunks[chunk].text.empty()) {
			width += _textLines[nrow].chunks[chunk].getFont()->getStringWidth(_textLines[nrow].chunks[chunk].text);
			mcol += _textLines[nrow].chunks[chunk].text.size();
		}

		if (width > x)
			break;
	}

	if (chunk == _textLines[nrow].chunks.size())
		chunk--;

	Common::U32String str = _textLines[nrow].chunks[chunk].text;

	ncol = mcol;

	for (int i = str.size(); i >= 0; i--) {
		int strw = _textLines[nrow].chunks[chunk].getFont()->getStringWidth(str);
		if (strw + pwidth < x) {
			ncol = pmcol + i;
			nsx = strw + pwidth;
			break;
		}

		str.deleteLastChar();
	}

	if (sx)
		*sx = nsx;
	if (sy)
		*sy = nsy;
	if (col)
		*col = ncol;
	if (row)
		*row = nrow;
}

Common::U32String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::U32String res;

	if (endRow == -1)
		endRow = _textLines.size() - 1;

	if (endCol == -1)
		endCol = getLineCharWidth(endRow);

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

//////////////////
// Text editing
void MacText::insertChar(byte c, int *row, int *col) {
	MacTextLine *line = &_textLines[*row];
	int pos = *col;
	uint ch = line->getChunkNum(&pos);

	for (ch = 0; ch < line->chunks.size(); ch++) {
		if (pos >= line->chunks[ch].text.size()) {
			pos -= line->chunks[ch].text.size();
		} else {
			break;
		}
	}

	if (ch == line->chunks.size()) {
		ch--;	// touch the last chunk
		pos = line->chunks[ch].text.size();
	}

	// We're in the needed chunk
	Common::U32String newchunk(line->chunks[ch].text);
	newchunk.insertChar(c, pos);
	int chunkw = line->chunks[ch].getFont()->getStringWidth(newchunk);
	int oldw = line->chunks[ch].getFont()->getStringWidth(line->chunks[ch].text);

	line->chunks[ch].text = newchunk;
	line->width = -1;	// Force recalc

	(*col)++;

	if (getLineWidth(*row) - oldw + chunkw > _maxWidth) { // Needs reshuffle
		reshuffleParagraph(row, col);
		_fullRefresh = true;
		recalcDims();
		render();
	} else {
		recalcDims();
		render(*row, *row);
	}
}

void MacText::deletePreviousChar(int *row, int *col) {
	if (*col == 0 && *row == 0) // nothing to do
		return;

	if (*col == 0) { // Need to glue the lines
		*col = getLineCharWidth(*row - 1);
		(*row)--;

		 // formatting matches, glue texts as normal
		if (_textLines[*row].lastChunk().equals(_textLines[*row + 1].firstChunk())) {
			_textLines[*row].lastChunk().text += _textLines[*row + 1].firstChunk().text;
			_textLines[*row + 1].firstChunk().text.clear();
		} else {
			// formatting doesn't match, move whole chunk
			_textLines[*row].chunks.push_back(MacFontRun(_textLines[*row + 1].firstChunk()));
			_textLines[*row].firstChunk().text.clear();
		}
		reshuffleParagraph(row, col);
	} else {
		int pos = *col - 1;
		uint ch = _textLines[*row].getChunkNum(&pos);

		_textLines[*row].chunks[ch].text.deleteChar(pos);

		(*col)--;

		reshuffleParagraph(row, col);
	}

	_fullRefresh = true;
	recalcDims();
	render();
}

void MacText::addNewLine(int *row, int *col) {
	MacTextLine *line = &_textLines[*row];
	int pos = *col;
	uint ch = line->getChunkNum(&pos);
	MacFontRun newchunk = line->chunks[ch];
	MacTextLine newline;

	newchunk.text = &line->chunks[ch].text.c_str()[pos];
	line->chunks[ch].text = Common::U32String(line->chunks[ch].text.c_str(), pos);
	newline.chunks.push_back(newchunk);

	for (uint i = ch + 1; i < line->chunks.size(); i++) {
		newline.chunks.push_back(MacFontRun(line->chunks[i]));
		line->chunks[i].text.clear();
	}
	line->width = -1; // Drop cache

	_textLines.insert_at(*row + 1, newline);

	(*row)++;
	*col = 0;

	_fullRefresh = true;
	recalcDims();
	render();
}

void MacText::reshuffleParagraph(int *row, int *col) {
}

} // End of namespace Graphics
