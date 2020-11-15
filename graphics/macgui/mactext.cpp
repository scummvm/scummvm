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
 *
 */

#include "common/unicode-bidi.h"
#include "common/timer.h"
#include "common/system.h"

#include "graphics/font.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {

enum {
	kConScrollStep = 12,

	kCursorMaxHeight = 100
};

static void cursorTimerHandler(void *refCon);

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
		if (pos >= (int)chunks[i].text.size()) {
			pos -= chunks[i].text.size();
		} else {
			break;
		}
	}

	if (i == chunks.size()) {
		i--;	// touch the last chunk
		pos = chunks[i].text.size();
	}

	*col = pos;

	return i;
}


MacText::MacText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, uint16 border, uint16 gutter, uint16 boxShadow, uint16 textShadow) :
	MacWidget(parent, x, y, w + 2, h, wm, true, border, gutter, boxShadow) {

	_str = s;
	_fullRefresh = true;

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
	_textShadow = textShadow;
	_maxWidth = maxWidth;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

MacText::MacText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::String &s, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, uint16 border, uint16 gutter, uint16 boxShadow, uint16 textShadow) :
	MacWidget(parent, x, y, w + 2, h, wm, true, border, gutter, boxShadow) {

	_str = Common::U32String(s);
	_fullRefresh = true;

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
	_textShadow = textShadow;
	_maxWidth = maxWidth;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

// NOTE: This constructor and the one afterward are for MacText engines that don't use widgets. This is the classic was MacText was constructed.
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0) {

	_str = s;
	_fullRefresh = true;

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
	_maxWidth = maxWidth;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

MacText::MacText(const Common::String &s, MacWindowManager *wm, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0) {

	_str = Common::U32String(s);
	_fullRefresh = true;

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
	_maxWidth = maxWidth;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

void MacText::init() {
	_defaultFormatting.wm = _wm;
	_currentFormatting = _defaultFormatting;
	_composeSurface->clear(_bgcolor);

	splitString(_str);
	recalcDims();

	_fullRefresh = true;
	_inTextSelection = false;

	_scrollPos = 0;
	_editable = false;
	_selectable = false;

	_editableRow = 0;

	_menu = nullptr;

	_cursorX = 0;
	_cursorY = 0;
	_cursorState = false;
	_cursorOff = false;

	_cursorRow = getLineCount() - 1;
	_cursorCol = getLineCharWidth(_cursorRow);

	_cursorRect = new Common::Rect(0, 0, 1, 0);

	_cursorSurface = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface->clear(_wm->_colorBlack);
	_cursorSurface2 = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface2->clear(_bgcolor);

	reallocSurface();
	setAlignOffset(_textAlignment);
	updateCursorPos();
	render();
}

MacText::~MacText() {
	_wm->setActiveWidget(nullptr);

	delete _cursorRect;
	delete _surface;
	delete _cursorSurface;
	delete _cursorSurface2;
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
		if (tolower(b) >= 'a')
			*res |= tolower(b) - 'a' + 10;
		else
			*res |= tolower(b) - '0';
	}

	return s;
}

// Adds the given string to the end of the last line/chunk
// while observing the _maxWidth and keeping this chunk's
// formatting
void MacText::chopChunk(const Common::U32String &str, int *curLinePtr) {
	int curLine = *curLinePtr;
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
		_textLines.insert_at(curLine, MacTextLine());
		_textLines[curLine].chunks.push_back(newchunk);

		D(9, "** chopChunk, added line: \"%s\"", toPrintable(text[i].encode()).c_str());
	}

	*curLinePtr = curLine;
}

void MacText::splitString(const Common::U32String &str, int curLine) {
	const Common::U32String::value_type *l = str.c_str();

	D(9, "** splitString(\"%s\")", toPrintable(str.encode()).c_str());

	if (_textLines.empty()) {
		_textLines.resize(1);
		_textLines[0].chunks.push_back(_defaultFormatting);
		D(9, "** splitString, added default formatting");
	} else {
		D(9, "** splitString, continuing, %d lines", _textLines.size());
	}

	if (str.empty()) {
		debug(9, "** splitString, empty line");
		return;
	}

	Common::U32String paragraph, tmp;

	if (curLine == -1)
		curLine = _textLines.size() - 1;

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

		D(9, "** splitString, paragraph: \"%s\"", Common::toPrintable(paragraph.encode()).c_str());

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
			chopChunk(tmp, &curLine);

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
		_textLines.insert_at(curLine, MacTextLine());
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
		_surface = new ManagedSurface(_textMaxWidth, _textMaxHeight, _wm->_pixelformat);

		return;
	}

	if (_surface->w < _textMaxWidth || _surface->h < _textMaxHeight) {
		// realloc surface and copy old content
		ManagedSurface *n = new ManagedSurface(_textMaxWidth, _textMaxHeight, _wm->_pixelformat);
		n->clear(_bgcolor);
		n->blitFrom(*_surface, Common::Point(0, 0));

		delete _surface;
		_surface = n;
	}
}

void MacText::render() {
	if (_fullRefresh) {
		_surface->clear(_bgcolor);
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
			debug(9, "MacText::render: line %d[%d] h:%d at %d,%d (%s) fontid: %d on %dx%d, color: %d",
				i, j, xOffset, _textLines[i].y, _textLines[i].height, _textLines[i].chunks[j].text.encode().c_str(),
				_textLines[i].chunks[j].fontId, _surface->w, _surface->h, _textLines[i].chunks[j].fgcolor);

			if (_textLines[i].chunks[j].text.empty())
				continue;

			//TODO: There might be a vertical alignment setting somewhere for differing font sizes in a single row?
			int yOffset = 0;
			if (_textLines[i].chunks[j].font->getFontHeight() < maxHeightForRow) {
				//TODO: determine where the magic value 2 comes from
				yOffset = maxHeightForRow - _textLines[i].chunks[j].font->getFontHeight() - 2;
			}

			_textLines[i].chunks[j].getFont()->drawString(_surface, convertBiDiU32String(_textLines[i].chunks[j].text), xOffset, _textLines[i].y + yOffset, w, _textLines[i].chunks[j].fgcolor);
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
			if (col >= (int)_textLines[line].chunks[i].text.size()) {
				col -= _textLines[line].chunks[i].text.size();
			} else {
				Common::U32String tmp = _textLines[line].chunks[i].text.substr(0, col);

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

void MacText::setAlignOffset(TextAlign align) {
	Common::Point offset;
	switch(align) {
	case kTextAlignLeft:
	default:
		offset = Common::Point(0, 0);
		break;
	case kTextAlignCenter:
		offset = Common::Point((_maxWidth / 2) - (_surface->w / 2), 0);
		break;
	case kTextAlignRight:
		offset = Common::Point(_maxWidth - (_surface->w + 1), 0);
		break;
	}

	if (offset != _alignOffset) {
		_contentIsDirty = true;
		_fullRefresh = true;
		_alignOffset = offset;
		_textAlignment = align;
	}
}

Common::Point MacText::calculateOffset() {
	return Common::Point(_alignOffset.x + _border + _gutter + 2, _alignOffset.y + _border + _gutter/2);
}

void MacText::setActive(bool active) {
	if (_active == active)
		return;

	MacWidget::setActive(active);

	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
	if (_active) {
		g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "macEditableText");
	}

	if (!_cursorOff && _cursorState == true)
		undrawCursor();
}

void MacText::setEditable(bool editable) {
	if (editable == _editable)
		return;

	_editable = editable;
	_cursorOff = !editable;

	setActive(editable);
	_active = editable;
	if (editable) {
		// TODO: Select whole region. This is done every time the text is set from
		// uneditable to editable.
		setSelection(0, true);
		setSelection(-1, false);

		_wm->setActiveWidget(this);
	} else {
		undrawCursor();
	}
}

void MacText::resize(int w, int h) {
	if (_surface->w == w && _surface->h == h)
		return;

	_maxWidth = w;
	setMaxWidth(_maxWidth);
}

void MacText::appendText(const Common::U32String &str, int fontId, int fontSize, int fontSlant, bool skipAdd) {
	appendTextDefault(str, skipAdd);

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

	_contentIsDirty = true;

	if (_editable) {
		_scrollPos = MAX(0, getTextHeight() - getDimensions().height());

		_cursorRow = getLineCount();
		_cursorCol = getLineCharWidth(_cursorRow);

		updateCursorPos();
	}
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
	_contentIsDirty = true;
	_textLines.clear();
	_str.clear();

	if (_surface)
		_surface->clear(_bgcolor);

	recalcDims();

	_cursorRow = _cursorCol = 0;
	updateCursorPos();
}

void MacText::removeLastLine() {
	if (!_textLines.size())
		return;

	int h = getLineHeight(_textLines.size() - 1) + _interLinear;

	_surface->fillRect(Common::Rect(0, _textMaxHeight - h, _surface->w, _textMaxHeight), _bgcolor);

	_textLines.pop_back();
	_textMaxHeight -= h;
}

void MacText::draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff) {
	if (_textLines.empty())
		return;

	render();

	g->blitFrom(*_surface, Common::Rect(MIN<int>(_surface->w, x), MIN<int>(_surface->h, y), MIN<int>(_surface->w, x + w), MIN<int>(_surface->h, y + h)), Common::Point(xoff, yoff));

	if (_textShadow)
		g->transBlitFrom(*_surface, Common::Rect(MIN<int>(_surface->w, x), MIN<int>(_surface->h, y), MIN<int>(_surface->w, x + w), MIN<int>(_surface->h, y + h)), Common::Point(xoff + _textShadow, yoff + _textShadow), 0xff);

	if (x + w < _surface->w || y + h < _surface->h)
		g->fillRect(Common::Rect(x, y, x + w, y + w), _bgcolor);

	_contentIsDirty = false;
	_cursorDirty = false;
}

bool MacText::draw(bool forceRedraw) {
	if (!needsRedraw() && !forceRedraw)
		return false;

	if (!_surface) {
		warning("MacText::draw: Null surface");
		return false;
	}

	// TODO: Clear surface fully when background colour changes.
	_contentIsDirty = false;
	_cursorDirty = false;

	Common::Point offset(calculateOffset());

	if (!_cursorState)
		_composeSurface->blitFrom(*_cursorSurface2, *_cursorRect, Common::Point(_cursorX, _cursorY + offset.y + 1));

	draw(_composeSurface, 0, _scrollPos, _surface->w, _scrollPos + _surface->h, offset.x, offset.y);

	for (int bb = 0; bb < _shadow; bb ++) {
		_composeSurface->hLine(_shadow, _composeSurface->h - _shadow + bb, _composeSurface->w, 0);
		_composeSurface->vLine(_composeSurface->w - _shadow + bb, _shadow, _composeSurface->h - _shadow, 0);
	}

	for (int bb = 0; bb < _border; bb++) {
		Common::Rect borderRect(bb, bb, _composeSurface->w - bb, _composeSurface->h - bb);
		_composeSurface->frameRect(borderRect, 0);
	}

	if (_cursorState)
		_composeSurface->blitFrom(*_cursorSurface, *_cursorRect, Common::Point(_cursorX, _cursorY + offset.y + 1));

	if (_selectedText.endY != -1 && _active)
		drawSelection();

	return true;
}

bool MacText::draw(ManagedSurface *g, bool forceRedraw) {
	if (!draw(forceRedraw))
		return false;

	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), Common::Point(_dims.left - 2, _dims.top - 2), _wm->_colorGreen2);

	return true;
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

void MacText::drawSelection() {
	if (_selectedText.endY == -1)
		return;

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startX, s.endX);
		SWAP(s.startY, s.endY);
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	int lastLineStart = s.endY;
	s.endY += getLineHeight(s.endRow);

	int start = s.startY - _scrollPos;
	start = MAX(0, start);

	if (start > getDimensions().height())
		return;

	int end = s.endY - _scrollPos;

	if (end < 0)
		return;

	end = MIN((int)getDimensions().height(), end);

	int numLines = 0;
	int x1 = 0, x2 = 0;

	for (int y = start; y < end; y++) {
		if (!numLines) {
			x1 = 0;
			x2 = getDimensions().width() - 1;

			if (y + _scrollPos == s.startY && s.startX > 0) {
				numLines = getLineHeight(s.startRow);
				x1 = s.startX;
			}
			if (y + _scrollPos >= lastLineStart) {
				numLines = getLineHeight(s.endRow);
				x2 = s.endX;
			}
		} else {
			numLines--;
		}

		byte *ptr = (byte *)_composeSurface->getBasePtr(x1, y);

		for (int x = x1; x < x2; x++, ptr++)
			if (*ptr == _fgcolor)
				*ptr = _bgcolor;
			else
				*ptr = _fgcolor;
	}
}

Common::U32String MacText::getSelection(bool formatted, bool newlines) {
	if (_selectedText.endY == -1)
		return Common::U32String();

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	return getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, formatted, newlines);
}

void MacText::clearSelection() {
	_selectedText.endY = _selectedText.startY = -1;
}

uint MacText::getSelectionIndex(bool start) {
	int pos = 0;

	if (!_inTextSelection && (_selectedText.startY < 0 && _selectedText.endY < 0))
		return pos;

	if (start) {
		for (int row = 0; row < _selectedText.startRow; row++)
			pos += getLineCharWidth(row);

		pos += _selectedText.startCol;
		return pos;
	} else {
		for (int row = 0; row < _selectedText.endRow; row++)
			pos += getLineCharWidth(row);

		pos += _selectedText.endCol;
		return pos;
	}
}

void MacText::setSelection(int pos, bool start) {
	int row = 0, col = 0;
	int colX = 0;

	if (pos > 0) {
		while (pos > 0) {
			if (pos < getLineCharWidth(row)) {
				for (uint i = 0; i < _textLines[row].chunks.size(); i++) {
					if ((uint)pos < _textLines[row].chunks[i].text.size()) {
						colX += _textLines[row].chunks[i].getFont()->getStringWidth(_textLines[row].chunks[i].text.substr(0, pos));
						col += pos + 1;
						pos = 0;
						break;
					} else {
						colX += _textLines[row].chunks[i].getFont()->getStringWidth(Common::U32String(_textLines[row].chunks[i].text));
						pos -= _textLines[row].chunks[i].text.size();
						col += _textLines[row].chunks[i].text.size() + 1;
					}
				}
				break;
			} else {
				pos -= getLineCharWidth(row) + 1; // (row ? 1 : 0);
			}

			row++;
			if ((uint)row >= _textLines.size()) {
				colX = _surface->w;
				col = getLineCharWidth(row);

				break;
			}
		}
	} else {
		row = _textLines.size() - 1;
		colX = _surface->w;
		col = getLineCharWidth(row);
	}

	int rowY = _textLines[row].y;

	if (start) {
		_selectedText.startX = colX;
		_selectedText.startY = rowY;
		_selectedText.startCol = col;
		_selectedText.startRow = row;
	} else {
		_selectedText.endX = colX;
		_selectedText.endY = rowY;
		_selectedText.endCol = col;
		_selectedText.endRow = row;
	}

	_contentIsDirty = true;
}

bool MacText::isCutAllowed() {
	if (_selectedText.startRow >= _editableRow &&
			_selectedText.endRow  >= _editableRow)
		return true;

	return false;
}

Common::U32String MacText::getEditedString() {
	return getTextChunk(_editableRow, 0, -1, -1);
}

Common::U32String MacText::cutSelection() {
	if (!isCutAllowed())
		return Common::U32String();

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	Common::U32String selection = MacText::getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, false, false);

	// TODO: Remove the actual text

	clearSelection();

	return selection;
}

bool MacText::processEvent(Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		if (!_editable)
			return false;

		setActive(true);

		if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
			return false;
		}

		switch (event.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (_cursorRow > 0 || _cursorCol > 0) {
				deletePreviousChar(&_cursorRow, &_cursorCol);
				updateCursorPos();
				_contentIsDirty = true;
			}
			return true;

		case Common::KEYCODE_RETURN:
			addNewLine(&_cursorRow, &_cursorCol);
			updateCursorPos();
			_contentIsDirty = true;
			return true;

		case Common::KEYCODE_LEFT:
			if (_cursorCol == 0) {
				if (_cursorRow == 0) { // Nowhere to go
					return true;
				}
				_cursorRow--;
				_cursorCol = getLineCharWidth(_cursorRow);
			} else {
				_cursorCol--;
			}
			updateCursorPos();

			return true;

		case Common::KEYCODE_RIGHT:
			if (_cursorCol >= getLineCharWidth(_cursorRow)) {
				if (_cursorRow == getLineCount() - 1) { // Nowhere to go
					return true;
				}
				_cursorRow++;
				_cursorCol = 0;
			} else {
				_cursorCol++;
			}
			updateCursorPos();

			return true;

		case Common::KEYCODE_UP:
			if (_cursorRow == 0)
				return true;

			_cursorRow--;

			getRowCol(_cursorX + 1, _textLines[_cursorRow].y, nullptr, nullptr, &_cursorRow, &_cursorCol);
			updateCursorPos();

			return true;

		case Common::KEYCODE_DOWN:
			if (_cursorRow == getLineCount() - 1)
				return true;

			_cursorRow++;

			getRowCol(_cursorX + 1, _textLines[_cursorRow].y, nullptr, nullptr, &_cursorRow, &_cursorCol);
			updateCursorPos();

			return true;

		case Common::KEYCODE_DELETE:
			// TODO
			warning("MacText::processEvent(): Delete is not yet implemented");
			return true;

		default:
			if (event.kbd.ascii == '~')
				return false;

			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				insertChar((byte)event.kbd.ascii, &_cursorRow, &_cursorCol);
				updateCursorPos();
				_contentIsDirty = true;

				return true;
			}

			break;
		}
	}

	if (event.type == Common::EVENT_WHEELUP) {
		scroll(-2);
		return true;
	}

	if (event.type == Common::EVENT_WHEELDOWN) {
		scroll(2);
		return true;
	}

	if (!_selectable)
		return false;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		_wm->setActiveWidget(this);

		startMarking(event.mouse.x, event.mouse.y);

		return true;
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_inTextSelection) {
			_inTextSelection = false;

			if (_selectedText.endY == -1 ||
					(_selectedText.endX == _selectedText.startX && _selectedText.endY == _selectedText.startY)) {
				_selectedText.startY = _selectedText.endY = -1;
				_contentIsDirty = true;

				if (_menu)
					_menu->enableCommand("Edit", "Copy", false);

				int x = event.mouse.x - getDimensions().left;
				int y = event.mouse.y - getDimensions().top + _scrollPos;

				getRowCol(x, y, nullptr, nullptr, &_cursorRow, &_cursorCol);
				updateCursorPos();
			} else {
				if (_menu) {
					_menu->enableCommand("Edit", "Copy", true);

					bool cutAllowed = isCutAllowed();

					_menu->enableCommand("Edit", "Cut", cutAllowed);
					_menu->enableCommand("Edit", "Clear", cutAllowed);
				}
			}
		}

		return true;
	} else if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_inTextSelection) {
			updateTextSelection(event.mouse.x, event.mouse.y);
			return true;
		}
	}

	return false;
}

void MacText::scroll(int delta) {
	int oldScrollPos = _scrollPos;

	_scrollPos += delta * kConScrollStep;

	if (_editable)
		_scrollPos = CLIP<int>(_scrollPos, 0, MacText::getTextHeight() - kConScrollStep);
	else
		_scrollPos = CLIP<int>(_scrollPos, 0, MAX(0, MacText::getTextHeight() - getDimensions().height()));

	undrawCursor();
	_cursorY -= (_scrollPos - oldScrollPos);
	_contentIsDirty = true;
}

void MacText::startMarking(int x, int y) {
	if (_textLines.size() == 0)
		return;

	x -= getDimensions().left - 2;
	y -= getDimensions().top;

	y += _scrollPos;

	getRowCol(x, y, &_selectedText.startX, &_selectedText.startY, &_selectedText.startRow, &_selectedText.startCol);

	_selectedText.endY = -1;

	_inTextSelection = true;
}

void MacText::updateTextSelection(int x, int y) {
	x -= getDimensions().left - 2;
	y -= getDimensions().top;

	y += _scrollPos;

	getRowCol(x, y, &_selectedText.endX, &_selectedText.endY, &_selectedText.endRow, &_selectedText.endCol);

	debug(3, "s: %d,%d (%d, %d) e: %d,%d (%d, %d)", _selectedText.startX, _selectedText.startY,
			_selectedText.startRow, _selectedText.startCol, _selectedText.endX,
			_selectedText.endY, _selectedText.endRow, _selectedText.endCol);

	_contentIsDirty = true;
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
	nsx = pwidth;

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

// If adjacent chunks have same format, then skip the format definition
// This happens when a long paragraph is split into several lines
#define ADDFORMATTING() \
	if (formatted) { \
		formatting = _textLines[i].chunks[chunk].toString(); \
		if (formatting != prevformatting) { \
			res += formatting; \
			prevformatting = formatting; \
		} \
	}

Common::U32String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::U32String res;

	if (endRow == -1)
		endRow = _textLines.size() - 1;

	if (endCol == -1)
		endCol = getLineCharWidth(endRow);

	if (_textLines.empty()) {
		return res;
	}

	startRow = CLIP(startRow, 0, (int)_textLines.size() - 1);
	endRow = CLIP(endRow, 0, (int)_textLines.size() - 1);

	Common::U32String formatting, prevformatting;

	for (int i = startRow; i <= endRow; i++) {
		// We requested only part of one line
		if (i == startRow && i == endRow) {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (_textLines[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				if (startCol <= 0) {
					ADDFORMATTING();

					if (endCol >= (int)_textLines[i].chunks[chunk].text.size())
						res += _textLines[i].chunks[chunk].text;
					else
						res += _textLines[i].chunks[chunk].text.substr(0, endCol);
				} else if ((int)_textLines[i].chunks[chunk].text.size() > startCol) {
					ADDFORMATTING();
					res += _textLines[i].chunks[chunk].text.substr(startCol, endCol - startCol);
				}

				startCol -= _textLines[i].chunks[chunk].text.size();
				endCol -= _textLines[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		// We are at the top line and it is not completely requested
		} else if (i == startRow && startCol != 0) {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (_textLines[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				if (startCol <= 0) {
					ADDFORMATTING();
					res += _textLines[i].chunks[chunk].text;
				} else if ((int)_textLines[i].chunks[chunk].text.size() > startCol) {
					ADDFORMATTING();
					res += _textLines[i].chunks[chunk].text.substr(startCol);
				}

				startCol -= _textLines[i].chunks[chunk].text.size();
			}
			if (newlines)
				res += '\n';
			else
				res += ' ';
		// We are at the end row, and it could be not completely requested
		} else if (i == endRow) {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (_textLines[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				ADDFORMATTING();

				if (endCol >= (int)_textLines[i].chunks[chunk].text.size())
					res += _textLines[i].chunks[chunk].text;
				else
					res += _textLines[i].chunks[chunk].text.substr(0, endCol);

				endCol -= _textLines[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		// We are in the middle of requested range, pass whole line
		} else {
			for (uint chunk = 0; chunk < _textLines[i].chunks.size(); chunk++) {
				if (_textLines[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				ADDFORMATTING();
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
	if (_textLines.empty()) {
		appendTextDefault(Common::String(c));
		(*col)++;

		return;
	}

	MacTextLine *line = &_textLines[*row];
	int pos = *col;
	uint ch = line->getChunkNum(&pos);

	Common::U32String newchunk(line->chunks[ch].text);

	if (pos >= (int)newchunk.size())
		newchunk += c;
	else
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

		for (uint i = 1; i < _textLines[*row + 1].chunks.size(); i++)
			_textLines[*row].chunks.push_back(MacFontRun(_textLines[*row + 1].chunks[i]));

		_textLines.remove_at(*row + 1);
	} else {
		int pos = *col - 1;
		uint ch = _textLines[*row].getChunkNum(&pos);

		if (pos == (int)_textLines[*row].chunks[ch].text.size())
			pos--;

		_textLines[*row].chunks[ch].text.deleteChar(pos);

		(*col)--;
	}

	_textLines[*row].width = -1; // flush the cache

	reshuffleParagraph(row, col);

	_fullRefresh = true;
	recalcDims();
	render();
}

void MacText::addNewLine(int *row, int *col) {
	if (_textLines.empty()) {
		appendTextDefault(Common::String("\n"));
		(*row)++;

		return;
	}

	MacTextLine *line = &_textLines[*row];
	int pos = *col;
	uint ch = line->getChunkNum(&pos);
	MacFontRun newchunk = line->chunks[ch];
	MacTextLine newline;

	newchunk.text = line->chunks[ch].text.substr(pos);
	line->chunks[ch].text = line->chunks[ch].text.substr(0, pos);
	newline.chunks.push_back(newchunk);

	for (uint i = ch + 1; i < line->chunks.size(); i++) {
		newline.chunks.push_back(MacFontRun(line->chunks[i]));
		line->chunks[i].text.clear();
	}
	line->width = -1; // Drop cache

	_textLines[*row].width = -1; // flush the cache

	_textLines.insert_at(*row + 1, newline);

	(*row)++;
	*col = 0;

	_fullRefresh = true;
	recalcDims();
	render();
}

void MacText::reshuffleParagraph(int *row, int *col) {
	// First, we looking for the paragraph start and end
	int start = *row, end = *row;

	while (start && !_textLines[start - 1].paragraphEnd)
		start--;

	while (end < (int)_textLines.size() - 1 && !_textLines[end].paragraphEnd) // stop at last line
		end++;

	// Get character pos within paragraph
	int ppos = 0;

	for (int i = start; i < *row; i++)
		ppos += getLineCharWidth(i);

	ppos += *col;

	// Get whole paragraph
	Common::U32String paragraph = getTextChunk(start, 0, end, -1, true, false);

	// Remove it from the text
	for (int i = start; i <= end; i++) {
		_textLines.remove_at(start);
	}

	// And now readd it
	splitString(paragraph, start);

	// Find new pos within paragraph after reshuffling
	*row = start;

	while (ppos > getLineCharWidth(*row)) {
		ppos -= getLineCharWidth(*row);
		(*row)++;
	}
	*col = ppos;
}

//////////////////
// Cursor stuff
static void cursorTimerHandler(void *refCon) {
	MacText *w = (MacText *)refCon;

	if (!w->_cursorOff)
		w->_cursorState = !w->_cursorState;

	w->_cursorDirty = true;
}

void MacText::updateCursorPos() {
	if (_textLines.empty()) {
		_cursorX = _cursorY = 0;
	} else {
		undrawCursor();

		_cursorRow = MIN<int>(_cursorRow, _textLines.size() - 1);

		Common::Point offset(calculateOffset());

		int alignOffset = 0;
		if (_textAlignment == kTextAlignRight)
			alignOffset = _textMaxWidth - getLineWidth(_cursorRow);
		else if (_textAlignment == kTextAlignCenter)
			alignOffset = (_textMaxWidth / 2) - (getLineWidth(_cursorRow) / 2);

		_cursorY = _textLines[_cursorRow].y + offset.y - 2;
		_cursorX = getLineWidth(_cursorRow, false, _cursorCol) + alignOffset + offset.x - 1;
	}

	int cursorHeight = getLineHeight(_cursorRow);

	if (cursorHeight == 0)
		cursorHeight = 12;

	// Do not exceed max height and widget height
	cursorHeight = MIN<int>(MIN<int>(cursorHeight, kCursorMaxHeight), _dims.height());

	_cursorRect->setHeight(cursorHeight);

	_cursorDirty = true;
}

void MacText::undrawCursor() {
	_cursorDirty = true;

	Common::Point offset(calculateOffset());
	_composeSurface->blitFrom(*_cursorSurface2, *_cursorRect, Common::Point(_cursorX, _cursorY + offset.y + 1));
}

} // End of namespace Graphics
