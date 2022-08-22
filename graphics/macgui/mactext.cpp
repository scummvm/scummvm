/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.

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

Common::CodePage MacFontRun::getEncoding() {
	return wm->_fontMan->getFontEncoding(fontId);
}

bool MacFontRun::plainByteMode() {
	Common::CodePage encoding = getEncoding();
	// This return statement accounts for utf8, invalid, and Mac Roman.
	// For future Unicode font compatibility, it should account for all codepages instead.
	return encoding != Common::kUtf8 && encoding != Common::kCodePageInvalid && encoding != Common::kMacRoman;
}

Common::String MacFontRun::getEncodedText() {
	Common::CodePage encoding = getEncoding();
	return Common::convertFromU32String(text, encoding);
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


MacText::MacText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, uint16 border, uint16 gutter, uint16 boxShadow, uint16 textShadow, bool fixedDims) :
	MacWidget(parent, x, y, w, h, wm, true, border, gutter, boxShadow),
	_macFont(macFont), _maxWidth(maxWidth), _textAlignment(textAlignment), _interLinear(interlinear) {

	D(6, "MacText::MacText(): fgcolor: %d, bgcolor: %d s: \"%s\"", fgcolor, bgcolor, Common::toPrintable(s.encode()).c_str());

	_str = s;
	_fullRefresh = true;

	_fixedDims = fixedDims;
	_wm = wm;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_textShadow = textShadow;
	_macFontMode = true;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
		_defaultFormatting.setValues(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

// NOTE: This constructor and the one afterward are for MacText engines that don't use widgets. This is the classic was MacText was constructed.
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *macFont, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, bool fixedDims) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0),
	_macFont(macFont), _maxWidth(maxWidth), _textAlignment(textAlignment), _interLinear(interlinear) {

	_str = s;

	_fixedDims = fixedDims;
	_wm = wm;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_textShadow = 0;
	_macFontMode = true;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

// Working with plain Font
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const Font *font, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, bool fixedDims) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0),
	_macFont(nullptr), _maxWidth(maxWidth), _textAlignment(textAlignment), _interLinear(interlinear) {

	_str = s;

	_fixedDims = fixedDims;
	_wm = wm;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_textShadow = 0;
	_macFontMode = false;

	if (font) {
		_defaultFormatting = MacFontRun(_wm, font, 0, font->getFontHeight(), 0, 0, 0);
		_defaultFormatting.font = font;
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

void MacText::init() {
	_fullRefresh = true;

	_textMaxWidth = 0;
	_textMaxHeight = 0;
	_surface = nullptr;
	_shadowSurface = nullptr;

	if (!_fixedDims) {
		int right = _dims.right;
		_dims.right = MAX<int>(_dims.right, _dims.left + _maxWidth + (2 * _border) + (2 * _gutter) + _shadow);
		if (right != _dims.right) {
			delete _composeSurface;
			_composeSurface = new ManagedSurface(_dims.width(), _dims.height(), _wm->_pixelformat);
		}
	}

	_selEnd = -1;
	_selStart = -1;

	_defaultFormatting.wm = _wm;

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

	_cursorRect = new Common::Rect(0, 0, 1, 1);

	// currently, we are not using fg color to render text. And we are not passing fg color correctly, thus we read it our self.
	MacFontRun colorFontRun = getFgColor();
	if (!colorFontRun.text.empty()) {
		_fgcolor = colorFontRun.fgcolor;
		colorFontRun.text.clear();
		debug(9, "Reading fg color though text, instead of the argument, read %x", _fgcolor);
		_defaultFormatting = colorFontRun;
		_defaultFormatting.wm = _wm;
	}

	_currentFormatting = _defaultFormatting;
	_composeSurface->clear(_bgcolor);

	_cursorSurface = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface->clear(_fgcolor);
	_cursorSurface2 = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface2->clear(_bgcolor);

	reallocSurface();
	setAlignOffset(_textAlignment);
	updateCursorPos();
	render();
}

MacText::~MacText() {
	if (_wm->getActiveWidget() == this)
		_wm->setActiveWidget(nullptr);

	delete _cursorRect;
	delete _surface;
	delete _shadowSurface;
	delete _cursorSurface;
	delete _cursorSurface2;
}

// this func returns the fg color of the first character we met in text
MacFontRun MacText::getFgColor() {
	if (_textLines.empty())
		return MacFontRun();
	for (uint i = 0; i < _textLines.size(); i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			if (!_textLines[i].chunks[j].text.empty())
				return _textLines[i].chunks[j];
		}
	}
	return MacFontRun();
}

// we are doing this because we may need to dealing with the plain byte. See ctor of mactext which contains String str instead of U32String str
// thus, if we are passing the str, meaning we are using plainByteMode. And when we calculate the string width. we need to convert it to it's original state first;
int MacText::getStringWidth(MacFontRun &format, const Common::U32String &str) {
	if (format.plainByteMode())
		return format.getFont()->getStringWidth(Common::convertFromU32String(str, format.getEncoding()));
	else
		return format.getFont()->getStringWidth(str);
}

void MacText::setMaxWidth(int maxWidth) {
	if (maxWidth == _maxWidth)
		return;

	if (maxWidth < 0) {
		warning("trying to set maxWidth to %d", maxWidth);
		return;
	}

	Common::U32String str = getTextChunk(0, 0, -1, -1, true, true);

	// keep the cursor pos
	int ppos = 0;
	for (int i = 0; i < _cursorRow; i++)
		ppos += getLineCharWidth(i);
	ppos += _cursorCol;

	_maxWidth = maxWidth;
	_textLines.clear();

	splitString(str);

	// restore the cursor pos
	_cursorRow = 0;
	while (ppos > getLineCharWidth(_cursorRow, true)) {
		ppos -= getLineCharWidth(_cursorRow, true);
		_cursorRow++;
	}
	_cursorCol = ppos;

	// after we set maxWidth, we reset the selection
	_selectedText.endY = -1;

	recalcDims();
	updateCursorPos();

	_fullRefresh = true;
	_contentIsDirty = true;
}

void MacText::setColors(uint32 fg, uint32 bg) {
	_bgcolor = bg;
	_fgcolor = fg;
	// also set the cursor color
	_cursorSurface->clear(_fgcolor);
	for (uint i = 0; i < _textLines.size(); i++)
		setTextColor(fg, i);

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::enforceTextFont(uint16 fontId) {
	for (uint i = 0; i < _textLines.size(); i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			_textLines[i].chunks[j].fontId = fontId;
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::setTextSize(int textSize) {
	for (uint i = 0; i < _textLines.size(); i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			_textLines[i].chunks[j].fontSize = textSize;
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::setTextColor(uint32 color, uint32 line) {
	if (line >= _textLines.size()) {
		warning("MacText::setTextColor(): line %d is out of bounds", line);
		return;
	}

	uint32 fgcol = _wm->findBestColor(color);
	for (uint j = 0; j < _textLines[line].chunks.size(); j++) {
		_textLines[line].chunks[j].fgcolor = fgcol;
	}

	// if we are calling this func separately, then here need a refresh
}

void MacText::getChunkPosFromIndex(int index, uint &lineNum, uint &chunkNum, uint &offset) {
	if (_textLines.empty()) {
		lineNum = chunkNum = offset = 0;
		return;
	}
	for (uint i = 0; i < _textLines.size(); i++) {
		if (getLineCharWidth(i) <= index) {
			index -= getLineCharWidth(i);
		} else {
			lineNum = i;
			chunkNum = _textLines[i].getChunkNum(&index);
			offset = index;
			return;
		}
	}
	lineNum = _textLines.size() - 1;
	chunkNum = _textLines[lineNum].chunks.size() - 1;
	offset = 0;
}

void setTextColorCallback(MacFontRun &macFontRun, int color) {
	macFontRun.fgcolor = color;
}

void MacText::setTextColor(uint32 color, uint32 start, uint32 end) {
	uint32 col = _wm->findBestColor(color);
	setTextChunks(start, end, col, setTextColorCallback);
}

void setTextSizeCallback(MacFontRun &macFontRun, int textSize) {
	macFontRun.fontSize = textSize;
}

void MacText::setTextSize(int textSize, int start, int end) {
	setTextChunks(start, end, textSize, setTextSizeCallback);
}

void MacText::setTextChunks(int start, int end, int param, void (*callback)(MacFontRun &, int)) {
	if (_textLines.empty())
		return;
	if (start > end)
		SWAP(start, end);

	uint startRow, startCol;
	uint endRow, endCol;
	uint offset;

	getChunkPosFromIndex(start, startRow, startCol, offset);
	// if offset != 0, then we need to split the chunk
	if (offset != 0) {
		uint textSize = _textLines[startRow].chunks[startCol].text.size();
		MacFontRun newChunk = _textLines[startRow].chunks[startCol];
		newChunk.text = newChunk.text.substr(offset, textSize - offset);
		_textLines[startRow].chunks[startCol].text = _textLines[startRow].chunks[startCol].text.substr(0, offset);
		_textLines[startRow].chunks.insert_at(startCol + 1, newChunk);
		startCol++;
	}

	getChunkPosFromIndex(end, endRow, endCol, offset);
	if (offset != 0) {
		uint textSize = _textLines[endRow].chunks[endCol].text.size();
		MacFontRun newChunk = _textLines[endRow].chunks[endCol];
		newChunk.text = newChunk.text.substr(offset, textSize - offset);
		_textLines[endRow].chunks[endCol].text = _textLines[endRow].chunks[endCol].text.substr(0, offset);
		_textLines[endRow].chunks.insert_at(endCol + 1, newChunk);
		endCol++;
	}

	for (uint i = startRow; i <= endRow; i++) {
		uint from, to;
		if (i == startRow && i == endRow) {
			from = startCol;
			to = endCol;
		} else if (i == startRow) {
			from = startCol;
			to = _textLines[startRow].chunks.size();
		} else if (i == endRow) {
			from = 0;
			to = endCol;
		} else {
			from = 0;
			to = _textLines[i].chunks.size();
		}
		for (uint j = from; j < to; j++) {
			callback(_textLines[i].chunks[j], param);
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void setTextFontCallback(MacFontRun &macFontRun, int fontId) {
	macFontRun.fontId = fontId;
}

void MacText::setTextFont(int fontId, int start, int end) {
	setTextChunks(start, end, fontId, setTextFontCallback);
}

void setTextSlantCallback(MacFontRun &macFontRun, int textSlant) {
	macFontRun.textSlant = textSlant;
}

void MacText::setTextSlant(int textSlant, int start, int end) {
	setTextChunks(start, end, textSlant, setTextSlantCallback);
}

void MacText::enforceTextSlant(int textSlant) {
	for (uint i = 0; i < _textLines.size(); i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			if (textSlant) {
				_textLines[i].chunks[j].textSlant |= textSlant;
			} else {
				_textLines[i].chunks[j].textSlant = textSlant;
			}
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

// this maybe need to amend
// currently, we just return the text size of first character.
int MacText::getTextSize(int start, int end) {
	return getTextChunks(start, end).fontSize;
}

uint32 MacText::getTextColor(int start, int end) {
	return getTextChunks(start, end).fgcolor;
}

int MacText::getTextFont(int start, int end) {
	return getTextChunks(start, end).fontId;
}

int MacText::getTextSlant(int start, int end) {
	return getTextChunks(start, end).textSlant;
}

// only getting the first chunk for the selected area
MacFontRun MacText::getTextChunks(int start, int end) {
	if (_textLines.empty())
		return _defaultFormatting;
	if (start > end)
		SWAP(start, end);

	uint startRow, startCol;
	uint offset;

	getChunkPosFromIndex(start, startRow, startCol, offset);
	return _textLines[startRow].chunks[startCol];
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
	D(9, "** chopChunk before wrap \"%s\"", Common::toPrintable(str.encode()).c_str());

	chunk->getFont()->wordWrapText(str, _maxWidth, text, w);

	if (text.size() == 0) {
		warning("chopChunk: too narrow width, >%d", _maxWidth);
		chunk->text += str;
		getLineCharWidth(curLine, true);

		return;
	}

	for (int i = 0; i < (int)text.size(); i++) {
		D(9, "** chopChunk result %d \"%s\"", i, toPrintable(text[i].encode()).c_str());
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

	// TODO::code is not elegant, we need to figure out a way which include all situations
	if (curLine == -1)
		curLine = _textLines.size();

	if (_textLines.empty()) {
		_textLines.resize(1);
		_textLines[0].chunks.push_back(_defaultFormatting);
		D(9, "** splitString, added default formatting");
	} else {
		_textLines.insert_at(curLine, MacTextLine());
		D(9, "** splitString, continuing, %d lines", _textLines.size());
	}

	if (curLine == -1)
		curLine = _textLines.size() - 1;

	if (str.empty()) {
		_textLines[curLine].chunks.push_back(_defaultFormatting);
		debug(9,"** splitString, empty line");
		return;
	}

	Common::U32String paragraph, tmp;

	MacFontRun current_format = _defaultFormatting;

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
			// deal with single \n
			if (*l == '\n') {
				l++;
				break;
			}

			paragraph += *l++;
		}

		D(9, "** splitString, paragraph: \"%s\"", Common::toPrintable(paragraph.encode()).c_str());

		// Now process whole paragraph
		const Common::U32String::value_type *s = paragraph.c_str();

		while (*s) {
			tmp.clear();

			// Skip \001
			if (*s == '\001') {
				s++;
				if (*s == '\001') {
					tmp += *s;
					s++;
				}
			}

			// get format
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

				current_format.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				if (!_macFontMode)
					current_format.font = _defaultFormatting.font;
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

				current_format.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

				// So far, we enforce single font here, though in the future, font size could be altered
				if (!_macFontMode)
					current_format.font = _defaultFormatting.font;
			}

			while (*s && *s != ' ' && *s != '\001') {
				tmp += *s;
				s++;
			}
			// meaning there is a word with multifont
			if (*s == '\001') {
				_textLines[curLine].chunks.push_back(current_format);
				_textLines[curLine].lastChunk().wordContinuation = true;
				_textLines[curLine].lastChunk().text = tmp;
				continue;
			}
			// calc word_width, the trick we define here is we don`t count the space
			int word_width = getStringWidth(current_format, tmp);
			// add all spaces left
			while (*s == ' ') {
				tmp += *s;
				s++;
			}

			// now let`s try to split
			// first we have to try to get the whole word
			Common::Array<MacFontRun> word;
			word.push_back(current_format);
			word[0].text = tmp;

			while (!_textLines[curLine].chunks.empty() && _textLines[curLine].lastChunk().wordContinuation) {
				word.push_back(_textLines[curLine].lastChunk());
				_textLines[curLine].chunks.pop_back();
			}

			for (int i = 1; i < (int)word.size(); i++) {
				word_width += getStringWidth(word[i], word[i].text);
				D(9, "** word \"%s\" textslant [%d]", Common::toPrintable(word[i].text.encode()).c_str(), word[i].textSlant);
			}

			int cur_width = getLineWidth(curLine, true);

			D(9, "curWidth %d word_width %d", cur_width, word_width);
			// if cur_width == 0 but there`s chunks, meaning there must be empty string here
			// if cur_width == 0, then you don`t have to add a newline for it
			if (cur_width + word_width >= _maxWidth && cur_width != 0) {
				++curLine;
				_textLines.insert_at(curLine, MacTextLine());
			}

			// deal with the super long word situation
			if (word_width > _maxWidth) {
				for (int i = word.size() - 1; i >= 0; i--) {
					cur_width = getLineWidth(curLine, true);
					// count the size without space
					// because you don`t want to split a word just for space
					// actually i think this part can be optimized because only word[0] have space
					// we just need to deal it specially

					// meaning you have to split this word;
					int tmp_width = 0;
					_textLines[curLine].chunks.push_back(word[i]);
					// empty the string
					_textLines[curLine].lastChunk().text = Common::U32String();
					for (Common::U32String::const_iterator it = word[i].text.begin(); it != word[i].text.end(); it++) {
						Common::U32String::unsigned_type c = *it;
						if (c == ' ') {
							// add the space left
							while (it != word[i].text.end()) {
								c = *it;
								_textLines[curLine].lastChunk().text += c;
								it++;
							}
							break;
						}

						// here, if we are in the plainByteMode, then we need to get the original text width, because current font may not resolve that u32string
						int char_width = 0;
						if (word[i].plainByteMode()) {
							char_width = word[i].getFont()->getCharWidth(Common::convertFromU32String(Common::U32String(it, 1), word[i].getEncoding())[0]);
						} else {
							char_width = word[i].getFont()->getCharWidth(c);
						}
						if (char_width + tmp_width + cur_width >= _maxWidth) {
							++curLine;
							_textLines.insert_at(curLine, MacTextLine());
							_textLines[curLine].chunks.push_back(word[i]);
							_textLines[curLine].lastChunk().text = Common::U32String();
							tmp_width = 0;
							cur_width = 0;
						}
						tmp_width += char_width;
						_textLines[curLine].lastChunk().text += c;
					}
				}
			} else {
				for (int i = word.size() - 1; i >= 0; i--) {
					_textLines[curLine].chunks.push_back(word[i]);
				}
			}

			// If it is end of the line, we're done
			if (!*s) {
				_textLines[curLine].paragraphEnd = true;
				D(9, "** splitString, end of line");
				break;
			}
		}

		if (!*l) { // If this is end of the string, we're done here
			break;
		}

		// Add new line
		D(9, "** splitString: new line");

		_textLines[curLine].paragraphEnd = true;
		// if the chunks is empty, which means the line will not be rendered properly
		// so we add a empty string here
		if (_textLines[curLine].chunks.empty()) {
			_textLines[curLine].chunks.push_back(_defaultFormatting);
		}

		curLine++;
		_textLines.insert_at(curLine, MacTextLine());
	}

#if DEBUG
	for (uint i = 0; i < _textLines.size(); i++) {
		debugN(9, "** splitString: %2d ", i);

		for (uint j = 0; j < _textLines[i].chunks.size(); j++)
			debugN(9, "[%d] \"%s\"", _textLines[i].chunks[j].text.size(), Common::toPrintable(_textLines[i].chunks[j].text.encode()).c_str());

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
		_surface = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);

		if (_textShadow)
			_shadowSurface = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);

		return;
	}

	if (_surface->w < _maxWidth || _surface->h < _textMaxHeight) {
		// realloc surface and copy old content
		ManagedSurface *n = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);
		n->clear(_bgcolor);
		n->blitFrom(*_surface, Common::Point(0, 0));

		delete _surface;
		_surface = n;

		// same as shadow surface
		if (_textShadow) {
			ManagedSurface *newShadowSurface = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);
			newShadowSurface->clear(_bgcolor);
			newShadowSurface->blitFrom(*_shadowSurface, Common::Point(0, 0));

			delete _shadowSurface;
			_shadowSurface = newShadowSurface;
		}
	}
}

void MacText::render() {
	if (_fullRefresh) {
		_surface->clear(_bgcolor);
		if (_textShadow)
			_shadowSurface->clear(_bgcolor);

		render(0, _textLines.size());

		_fullRefresh = false;
	}
}

void MacText::render(int from, int to, int shadow) {
	int w = MIN(_maxWidth, _textMaxWidth);
	ManagedSurface *surface = shadow ? _shadowSurface : _surface;

	int myFrom = from, myTo = to + 1, delta = 1;

	if (_wm->_language == Common::HE_ISR) {
		myFrom = to;
		myTo = from - 1;
		delta = -1;
	}

	for (int i = myFrom; i != myTo; i += delta) {
		int xOffset = getAlignOffset(i);
		xOffset++;

		int start = 0, end = _textLines[i].chunks.size();
		if (_wm->_language == Common::HE_ISR) {
			start = _textLines[i].chunks.size() - 1;
			end = -1;
		}

		int maxAscentForRow = 0;
		for (int j = start; j != end; j += delta) {
			if (_textLines[i].chunks[j].font->getFontAscent() > maxAscentForRow)
				maxAscentForRow = _textLines[i].chunks[j].font->getFontAscent();
		}

		// TODO: _textMaxWidth, when -1, was not rendering ANY text.
		for (int j = start; j != end; j += delta) {
			debug(9, "MacText::render: line %d[%d] h:%d at %d,%d (%s) fontid: %d on %dx%d, fgcolor: %d bgcolor: %d, font: %p",
				  i, j, _textLines[i].height, xOffset, _textLines[i].y, _textLines[i].chunks[j].text.encode().c_str(),
				  _textLines[i].chunks[j].fontId, _surface->w, _surface->h, _textLines[i].chunks[j].fgcolor, _bgcolor,
				  (const void *)_textLines[i].chunks[j].getFont());

			if (_textLines[i].chunks[j].text.empty())
				continue;

			int yOffset = 0;
			if (_textLines[i].chunks[j].font->getFontAscent() < maxAscentForRow) {
				yOffset = maxAscentForRow - _textLines[i].chunks[j].font->getFontAscent();
			}

			if (_textLines[i].chunks[j].plainByteMode()) {
				Common::String str = _textLines[i].chunks[j].getEncodedText();
				_textLines[i].chunks[j].getFont()->drawString(surface, str, xOffset, _textLines[i].y + yOffset, w, shadow ? _wm->_colorBlack : _textLines[i].chunks[j].fgcolor, Graphics::kTextAlignLeft, 0, true);
				xOffset += _textLines[i].chunks[j].getFont()->getStringWidth(str);
			} else {
				if (_wm->_language == Common::HE_ISR)
					_textLines[i].chunks[j].getFont()->drawString(surface, convertBiDiU32String(_textLines[i].chunks[j].text, Common::BIDI_PAR_RTL), xOffset, _textLines[i].y + yOffset, w, shadow ? _wm->_colorBlack : _textLines[i].chunks[j].fgcolor, Graphics::kTextAlignLeft, 0, true);
				else
					_textLines[i].chunks[j].getFont()->drawString(surface, convertBiDiU32String(_textLines[i].chunks[j].text), xOffset, _textLines[i].y + yOffset, w, shadow ? _wm->_colorBlack : _textLines[i].chunks[j].fgcolor, Graphics::kTextAlignLeft, 0, true);
				xOffset += _textLines[i].chunks[j].getFont()->getStringWidth(_textLines[i].chunks[j].text);
			}
		}
	}
}

void MacText::render(int from, int to) {
	if (_textLines.empty())
		return;

	reallocSurface();

	from = MAX<int>(0, from);
	to = MIN<int>(to, _textLines.size() - 1);

	// Clear the screen
	_surface->fillRect(Common::Rect(0, _textLines[from].y, _surface->w, _textLines[to].y + getLineHeight(to)), _bgcolor);

	// render the shadow surface;
	if (_textShadow)
		render(from, to, _textShadow);

	render(from, to, 0);

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
		if (enforce && _macFontMode)
			_textLines[line].chunks[i].font = nullptr;

		if (col >= 0) {
			if (col >= (int)_textLines[line].chunks[i].text.size()) {
				col -= _textLines[line].chunks[i].text.size();
			} else {
				Common::U32String tmp = _textLines[line].chunks[i].text.substr(0, col);

				width += getStringWidth(_textLines[line].chunks[i], tmp);

				return width;
			}
		}

		if (!_textLines[line].chunks[i].text.empty()) {
			width += getStringWidth(_textLines[line].chunks[i], _textLines[line].chunks[i].text);
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

int MacText::getLastLineWidth() {
	if (_textLines.size() == 0)
		return 0;

	return getLineWidth(_textLines.size() - 1, true);
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
	_fullRefresh = true;
	render();
	_contentIsDirty = true;
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
		y += MAX(getLineHeight(i), _interLinear);
	}

	_textMaxHeight = y;

	if (!_fixedDims) {
		int newBottom = _dims.top + _textMaxHeight + (2 * _border) + _gutter + _shadow;
		if (newBottom > _dims.bottom) {
			_dims.bottom = newBottom;
			delete _composeSurface;
			_composeSurface = new ManagedSurface(_dims.width(), _dims.height(), _wm->_pixelformat);
			reallocSurface();
			if (!_fullRefresh) {
				_fullRefresh = true;
				render();
			}
			_fullRefresh = true;
			_contentIsDirty = true;
		}
	}
}

void MacText::setAlignOffset(TextAlign align) {
	if (_textAlignment == align)
		return;

	_contentIsDirty = true;
	_fullRefresh = true;
	_textAlignment = align;
}

Common::Point MacText::calculateOffset() {
	return Common::Point(_border + _gutter, _border + _gutter / 2);
}

void MacText::setSelRange(int selStart, int selEnd) {
	if (selStart == _selStart && selEnd == _selEnd)
		return;
	_selStart = selStart;
	_selEnd = selEnd;
}

void MacText::setActive(bool active) {
	if (_active == active)
		return;

	MacWidget::setActive(active);

	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
	if (_active) {
		g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "macEditableText");
		// inactive -> active, we reset the selection
		setSelection(_selStart, true);
		setSelection(_selEnd, false);
	} else {
		// clear the selection and cursor
		_selectedText.endY = -1;
		_cursorState = false;
		_inTextSelection = false;
	}

	// after we change the status of active, we need to do a refresh to clear the stuff we don't need
	_contentIsDirty = true;

	if (!_cursorOff && _cursorState == true)
		undrawCursor();
}

void MacText::setEditable(bool editable) {
	if (editable == _editable)
		return;

	// if we are not editable, then we also update the state of active, and tell wm too
	if (!editable) {
		setActive(false);
		if (_wm->getActiveWidget() == this)
			_wm->setActiveWidget(nullptr);
	}

	_editable = editable;
	_cursorOff = !editable;
	_selectable = editable;
	_focusable = editable;

	if (!editable) {
		undrawCursor();
	}
}

void MacText::resize(int w, int h) {
	if (_surface->w == w && _surface->h == h)
		return;

	setMaxWidth(w);
	if (_composeSurface->w != w || _composeSurface->h != h) {
		delete _composeSurface;
		_composeSurface = new Graphics::ManagedSurface(w, h);
		_dims.right = _dims.left + w;
		_dims.bottom = _dims.top + h;

		_contentIsDirty = true;
		_fullRefresh = true;
	}
}

void MacText::appendText(const Common::U32String &str, int fontId, int fontSize, int fontSlant, bool skipAdd) {
	appendText(str, fontId, fontSize, fontSlant, 0, 0, 0, skipAdd);
}

void MacText::appendText(const Common::U32String &str, int fontId, int fontSize, int fontSlant, uint16 r, uint16 g, uint16 b, bool skipAdd) {
	uint oldLen = _textLines.size();

	MacFontRun fontRun = MacFontRun(_wm, fontId, fontSlant, fontSize, r, g, b);

	_currentFormatting = fontRun;

	// we check _str here, if _str is empty but _textLines is not empty, and they are not the end of paragraph
	// then we remove those empty lines
	// too many special check may cause some strange problem in the future
	if (_str.empty()) {
		while (!_textLines.empty() && !_textLines.back().paragraphEnd)
			removeLastLine();
	}

	// we need to split the string with the font, in order to get the correct font
	Common::U32String strWithFont = Common::U32String(fontRun.toString()) + str;

	if (!skipAdd)
		_str += strWithFont;

	appendText_(strWithFont, oldLen);
}

void MacText::appendText(const Common::U32String &str, const Font *font, uint16 r, uint16 g, uint16 b, bool skipAdd) {
	uint oldLen = _textLines.size();

	MacFontRun fontRun = MacFontRun(_wm, font, 0, font->getFontHeight(), r, g, b);

	_currentFormatting = fontRun;

	if (_str.empty()) {
		while (!_textLines.empty() && !_textLines.back().paragraphEnd)
			removeLastLine();
	}

	Common::U32String strWithFont = Common::U32String(fontRun.toString()) + str;

	if (!skipAdd)
		_str += strWithFont;

	appendText_(strWithFont, oldLen);
}

void MacText::appendText_(const Common::U32String &strWithFont, uint oldLen) {
	splitString(strWithFont);
	recalcDims();

	render(oldLen - 1, _textLines.size());

	_contentIsDirty = true;

	if (_editable) {
		_scrollPos = MAX<int>(0, getTextHeight() - getDimensions().height());

		_cursorRow = getLineCount();
		_cursorCol = getLineCharWidth(_cursorRow);

		updateCursorPos();
	}
}

void MacText::appendTextDefault(const Common::U32String &str, bool skipAdd) {
	uint oldLen = _textLines.size();

	_currentFormatting = _defaultFormatting;
	Common::U32String strWithFont = Common::U32String(_defaultFormatting.toString()) + str;

	if (!skipAdd) {
		_str += strWithFont;
	}
	splitString(strWithFont);
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

	if (x + w < _surface->w || y + h < _surface->h)
		g->fillRect(Common::Rect(x + xoff, y + yoff, x + w + xoff, y + h + yoff), _bgcolor);

	// blit shadow surface first
	if (_textShadow)
		g->blitFrom(*_shadowSurface, Common::Rect(MIN<int>(_surface->w, x), MIN<int>(_surface->h, y), MIN<int>(_surface->w, x + w), MIN<int>(_surface->h, y + h)), Common::Point(xoff + _textShadow, yoff + _textShadow));

	g->transBlitFrom(*_surface, Common::Rect(MIN<int>(_surface->w, x), MIN<int>(_surface->h, y), MIN<int>(_surface->w, x + w), MIN<int>(_surface->h, y + h)), Common::Point(xoff, yoff), _bgcolor);

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

	// we need to find out a way to judge whether we need to clear the surface
	// currently, we just use the _contentIsDirty
	if (_contentIsDirty)
		_composeSurface->clear(_bgcolor);

	// TODO: Clear surface fully when background colour changes.
	_cursorDirty = false;

	Common::Point offset(calculateOffset());

	// if we are drawing the selection text or we are selecting, then we don't draw the cursor
	if (!((_inTextSelection || _selectedText.endY != -1) && _active)) {
		if (!_cursorState)
			_composeSurface->blitFrom(*_cursorSurface2, *_cursorRect, Common::Point(_cursorX + offset.x, _cursorY + offset.y));
		else
			_composeSurface->blitFrom(*_cursorSurface, *_cursorRect, Common::Point(_cursorX + offset.x, _cursorY + offset.y));
	}

	if (!(_contentIsDirty || forceRedraw))
		return true;

	draw(_composeSurface, 0, _scrollPos, _surface->w, _scrollPos + _surface->h, offset.x, offset.y);

	for (int bb = 0; bb < _shadow; bb++) {
		_composeSurface->hLine(_shadow, _composeSurface->h - _shadow + bb, _composeSurface->w, 0);
		_composeSurface->vLine(_composeSurface->w - _shadow + bb, _shadow, _composeSurface->h - _shadow, 0);
	}

	for (int bb = 0; bb < _border; bb++) {
		Common::Rect borderRect(bb, bb, _composeSurface->w - bb, _composeSurface->h - bb);
		_composeSurface->frameRect(borderRect, 0);
	}

	if (_selectedText.endY != -1)
		drawSelection(offset.x, offset.y);

	_contentIsDirty = false;

	return true;
}

bool MacText::draw(ManagedSurface *g, bool forceRedraw) {
	if (!draw(forceRedraw))
		return false;

	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), Common::Point(_dims.left, _dims.top), _wm->_colorGreen2);

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

void MacText::drawSelection(int xoff, int yoff) {
	if (_selectedText.endY == -1)
		return;

	// we check if the selection size is 0, then we don't draw it anymore, and we set the cursor here
	// it's a small optimize, but can bring us correct behavior
	if (!_inTextSelection && _selectedText.startX == _selectedText.endX && _selectedText.startY == _selectedText.endY) {
		_cursorRow = _selectedText.startRow;
		_cursorCol = _selectedText.startCol;
		updateCursorPos();
		_selectedText.startY = _selectedText.endY = -1;
		return;
	}

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

	int maxSelectionHeight = getDimensions().height() - _border - _gutter / 2;
	int maxSelectionWidth = getDimensions().width() - _border - _gutter;

	if (s.endCol == getLineCharWidth(s.endRow))
		s.endX = maxSelectionWidth;

	end = MIN((int)maxSelectionHeight, end);

	// if we are selecting all text, then we invert the whole area
	if ((uint)s.endRow == _textLines.size() - 1)
		end = maxSelectionHeight;

	int numLines = 0;
	int x1 = 0, x2 = maxSelectionWidth;
	int row = s.startRow;
	int alignOffset = 0;

	// we may draw part of the selection, so we need to calc the height of first line
	if (s.startY < _scrollPos) {
		int start_row = 0;
		getRowCol(s.startX, _scrollPos, nullptr, &numLines, &start_row, nullptr);
		numLines = getLineHeight(start_row) - (_scrollPos - numLines);
		if (start_row == s.startRow)
			x1 = s.startX;
		if (start_row == s.endRow)
			x2 = s.endX;
		// deal with the first line, which is not a complete line
		if (numLines) {
			alignOffset = getAlignOffset(start_row);

			if (start_row == s.startRow && s.startCol != 0) {
				x1 = MIN<int>(x1 + xoff + alignOffset, maxSelectionWidth);
				x2 = MIN<int>(x2 + xoff + alignOffset, maxSelectionWidth);
			} else {
				x1 = MIN<int>(x1 + xoff, maxSelectionWidth);
				x2 = MIN<int>(x2 + xoff + alignOffset, maxSelectionWidth);
			}

			row = start_row + 1;
		}
	}

	end = MIN(end, maxSelectionHeight - yoff);
	for (int y = start; y < end; y++) {
		if (!numLines && (uint)row < _textLines.size()) {
			x1 = 0;
			x2 = maxSelectionWidth;

			alignOffset = getAlignOffset(row);

			numLines = getLineHeight(row);
			if (y + _scrollPos == s.startY && s.startX > 0)
				x1 = s.startX;
			if (y + _scrollPos >= lastLineStart)
				x2 = s.endX;

			// if we are selecting text reversely, and we are at the first line but not the select from begining, then we add offset to x1
			// the reason here is if we are not drawing the single line, then we draw selection from x1 to x2 + offset. i.e. we draw from begin
			// otherwise, we draw selection from x1 + offset to x2 + offset
			if (row == s.startRow && s.startCol != 0) {
				x1 = MIN<int>(x1 + xoff + alignOffset, maxSelectionWidth);
				x2 = MIN<int>(x2 + xoff + alignOffset, maxSelectionWidth);
			} else {
				x1 = MIN<int>(x1 + xoff, maxSelectionWidth);
				x2 = MIN<int>(x2 + xoff + alignOffset, maxSelectionWidth);
			}
			row++;
		}
		numLines--;

		byte *ptr = (byte *)_composeSurface->getBasePtr(x1, MIN<int>(y + yoff, maxSelectionHeight - 1));

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
	// -1 for start represent the begining of text, i.e. 0
	if (pos == -1 && start)
		pos = 0;
	int row = 0, col = 0;
	int colX = 0;

	if (pos > 0) {
		while (pos > 0) {
			if (pos < getLineCharWidth(row)) {
				for (uint i = 0; i < _textLines[row].chunks.size(); i++) {
					if ((uint)pos < _textLines[row].chunks[i].text.size()) {
						colX += getStringWidth(_textLines[row].chunks[i], _textLines[row].chunks[i].text.substr(0, pos));
						col += pos;
						pos = 0;
						break;
					} else {
						colX += getStringWidth(_textLines[row].chunks[i], _textLines[row].chunks[i].text);
						pos -= _textLines[row].chunks[i].text.size();
						col += _textLines[row].chunks[i].text.size();
					}
				}
				break;
			} else {
				pos -= getLineCharWidth(row); // (row ? 1 : 0);
			}

			row++;
			if ((uint)row >= _textLines.size()) {
				row = _textLines.size() - 1;
				colX = _surface->w;
				col = getLineCharWidth(row);

				break;
			}
		}
	} else if (pos == 0) {
		colX = col = row = 0;
	} else {
		row = _textLines.size() - 1;
		col = getLineCharWidth(row);
		// if we don't have any text, then we won't select the whole area.
		if (_textMaxWidth == 0)
			colX = 0;
		else
			colX = _textMaxWidth;
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

Common::U32String MacText::getPlainText() {
	Common::U32String res;
	for (uint i = 0; i < _textLines.size(); i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			res += _textLines[i].chunks[j].text;
		}
	}

	return res;
}

Common::U32String MacText::cutSelection() {
	if (!isCutAllowed())
		return Common::U32String();

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	Common::U32String selection = MacText::getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, true, true);

	deleteSelection();
	clearSelection();

	return selection;
}

bool MacText::processEvent(Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		if (!_editable)
			return false;

		setActive(true);

		if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_x:
				_wm->setTextInClipboard(cutSelection());
				return true;
			case Common::KEYCODE_c:
				_wm->setTextInClipboard(getSelection(true, true));
				return true;
			case Common::KEYCODE_v:
				if (g_system->hasTextInClipboard()) {
					if (_selectedText.endY != -1)
						cutSelection();
					insertTextFromClipboard();
				}
				return true;
			default:
				break;
			}
			return false;
		}

		switch (event.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			// if we have the selectedText, then we delete it
			if (_selectedText.endY != -1) {
				cutSelection();
				_contentIsDirty = true;
				return true;
			}
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

			getRowCol(_cursorX, _textLines[_cursorRow].y, nullptr, nullptr, &_cursorRow, &_cursorCol);
			updateCursorPos();

			return true;

		case Common::KEYCODE_DOWN:
			if (_cursorRow == getLineCount() - 1)
				return true;

			_cursorRow++;

			getRowCol(_cursorX, _textLines[_cursorRow].y, nullptr, nullptr, &_cursorRow, &_cursorCol);
			updateCursorPos();

			return true;

		case Common::KEYCODE_DELETE:
			// first try to delete the selected text
			if (_selectedText.endY != -1) {
				cutSelection();
				_contentIsDirty = true;
				return true;
			}
			// move cursor to next one and delete previous char
			if (_cursorCol >= getLineCharWidth(_cursorRow)) {
				if (_cursorRow == getLineCount() - 1) {
					return true;
				}
				_cursorRow++;
				_cursorCol = 0;
			} else {
				_cursorCol++;
			}
			deletePreviousChar(&_cursorRow, &_cursorCol);
			updateCursorPos();
			_contentIsDirty = true;
			return true;

		default:
			if (event.kbd.ascii == '~')
				return false;

			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				// if we have selected text, then we delete it, then we try to insert char
				if (_selectedText.endY != -1) {
					cutSelection();
					_contentIsDirty = true;
				}
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
		bool active = _active;
		_wm->setActiveWidget(this);
		if (active == true) {
			// inactive -> active switching, we don't start marking the selection, because we have initial selection
			startMarking(event.mouse.x, event.mouse.y);
		}

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

				Common::Point offset = calculateOffset();
				int x = event.mouse.x - getDimensions().left - offset.x;
				int y = event.mouse.y - getDimensions().top + _scrollPos - offset.y;

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
	// disable the scroll for auto expand text
	// should be amend to check the text type. e.g. auto expand type, fixed type, scroll type.
	if (!_fixedDims)
		return;

	int oldScrollPos = _scrollPos;

	_scrollPos += delta * kConScrollStep;

	if (_editable)
		_scrollPos = CLIP<int>(_scrollPos, 0, MacText::getTextHeight() - kConScrollStep);
	else
		_scrollPos = CLIP<int>(_scrollPos, 0, MAX<int>(0, MacText::getTextHeight() - getDimensions().height()));

	undrawCursor();
	_cursorY -= (_scrollPos - oldScrollPos);
	_contentIsDirty = true;
}

void MacText::startMarking(int x, int y) {
	if (_textLines.size() == 0)
		return;

	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;

	y += _scrollPos;

	getRowCol(x, y, &_selectedText.startX, &_selectedText.startY, &_selectedText.startRow, &_selectedText.startCol);

	_selectedText.endY = -1;

	_inTextSelection = true;
}

void MacText::updateTextSelection(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;

	y += _scrollPos;

	getRowCol(x, y, &_selectedText.endX, &_selectedText.endY, &_selectedText.endRow, &_selectedText.endCol);

	debug(3, "s: %d,%d (%d, %d) e: %d,%d (%d, %d)", _selectedText.startX, _selectedText.startY,
			_selectedText.startRow, _selectedText.startCol, _selectedText.endX,
			_selectedText.endY, _selectedText.endRow, _selectedText.endCol);

	_contentIsDirty = true;
}

int MacText::getMouseChar(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;
	y += _scrollPos;

	int dx, dy, row, col;
	getRowCol(x, y, &dx, &dy, &row, &col);

	int index = 0;
	for (int r = 0; r < row; r++)
		index += getLineCharWidth(r);
	index += col;

	return index + 1;
}

int MacText::getMouseWord(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;
	y += _scrollPos;

	int dx, dy, row, col;
	getRowCol(x, y, &dx, &dy, &row, &col);

	int index = 0;
	for (int i = 0; i < row; i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			if (_textLines[i].chunks[j].text.empty())
				continue;
			index++;
		}
	}

	int cur = 0;
	for (uint j = 0; j < _textLines[row].chunks.size(); j++) {
		if (_textLines[row].chunks[j].text.empty())
			continue;
		cur += _textLines[row].chunks[j].text.size();
		if (cur <= col)
			index++;
		else
			break;
	}

	return index + 1;
}

int MacText::getMouseItem(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;
	y += _scrollPos;

	int dx, dy, row, col;
	getRowCol(x, y, &dx, &dy, &row, &col);

	int index = 0;
	for (int i = 0; i < row; i++) {
		for (uint j = 0; j < _textLines[i].chunks.size(); j++) {
			if (_textLines[i].chunks[j].text.empty())
				continue;
			if (_textLines[i].chunks[j].getEncodedText().contains(','))
				index++;
		}
	}

	int cur = 0;
	for (uint i = 0; i < _textLines[row].chunks.size(); i++) {
		if (_textLines[row].chunks[i].text.empty())
			continue;

		for (uint j = 0; j < _textLines[row].chunks[i].text.size(); j++) {
			cur++;
			if (cur > col)
				break;
			if (_textLines[row].chunks[i].text[j] == ',')
				index++;
		}

		if (cur > col)
			break;
	}

	return index + 1;
}

int MacText::getMouseLine(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;
	y += _scrollPos;

	int dx, dy, row, col;
	getRowCol(x, y, &dx, &dy, &row, &col);

	return row + 1;
}

int MacText::getAlignOffset(int row) {
	int alignOffset = 0;
	if (_textAlignment == kTextAlignRight)
		alignOffset = MAX<int>(0, _maxWidth - getLineWidth(row) - 1);
	else if (_textAlignment == kTextAlignCenter)
		alignOffset = (_maxWidth / 2) - (getLineWidth(row) / 2);
	return alignOffset;
}

void MacText::getRowCol(int x, int y, int *sx, int *sy, int *row, int *col) {
	int nsx, nsy, nrow, ncol;

	if (y > _textMaxHeight) {
		x = _surface->w;
	}

	y = CLIP(y, 0, _textMaxHeight);

	nrow = _textLines.size();
	// use [lb, ub) bsearch here, final answer would be lb
	int lb = 0, ub = nrow;
	while (ub - lb > 1) {
		int mid = (ub + lb) / 2;
		if (_textLines[mid].y <= y) {
			lb = mid;
		} else {
			ub = mid;
		}
	}
	nrow = lb;

	nsy = _textLines[nrow].y;

	ncol = 0;

	int alignOffset = getAlignOffset(nrow);

	int width = 0, pwidth = 0;
	int mcol = 0, pmcol = 0;
	uint chunk;
	for (chunk = 0; chunk < _textLines[nrow].chunks.size(); chunk++) {
		pwidth = width;
		pmcol = mcol;
		if (!_textLines[nrow].chunks[chunk].text.empty()) {
			width += getStringWidth(_textLines[nrow].chunks[chunk], _textLines[nrow].chunks[chunk].text);
			mcol += _textLines[nrow].chunks[chunk].text.size();
		}

		if (width + alignOffset > x)
			break;
	}

	if (chunk == _textLines[nrow].chunks.size())
		chunk--;

	// prevent out bounding error, because sometimes chunk.size() is 0, thus we don't have correct chunk number.
	if (chunk < _textLines[nrow].chunks.size()) {
		Common::U32String str = _textLines[nrow].chunks[chunk].text;

		ncol = mcol;
		nsx = pwidth;

		for (int i = str.size(); i >= 0; i--) {
			int strw = getStringWidth(_textLines[nrow].chunks[chunk], str);
			if (strw + pwidth + alignOffset <= x) {
				ncol = pmcol + i;
				nsx = strw + pwidth;
				break;
			}

			str.deleteLastChar();
		}
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
#define ADDFORMATTING()                                                                      \
	if (formatted) {                                                                         \
		formatting = Common::U32String(_textLines[i].chunks[chunk].toString()); \
		if (formatting != prevformatting) {                                                  \
			res += formatting;                                                               \
			prevformatting = formatting;                                                     \
		}                                                                                    \
	}

Common::U32String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::U32String res("");

	if (endRow == -1)
		endRow = _textLines.size() - 1;

	if (endCol == -1)
		endCol = getLineCharWidth(endRow);
	if (_textLines.empty()) {
		return res;
	}

	startRow = CLIP(startRow, 0, (int)_textLines.size() - 1);
	endRow = CLIP(endRow, 0, (int)_textLines.size() - 1);

	Common::U32String formatting(""), prevformatting("");

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
			if (newlines && _textLines[i].paragraphEnd)
				res += '\n';
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

			if (newlines && _textLines[i].paragraphEnd)
				res += '\n';
		}
	}

	return res;
}

// mostly, we refering reshuffleParagraph to implement this function
void MacText::insertTextFromClipboard() {
	int ppos = 0;
	Common::U32String str = _wm->getTextFromClipboard(Common::U32String(_defaultFormatting.toString()), &ppos);

	if (_textLines.empty()) {
		splitString(str, 0);
	} else {
		int start = _cursorRow, end = _cursorRow;

		while (start && !_textLines[start - 1].paragraphEnd)
			start--;

		while (end < (int)_textLines.size() - 1 && !_textLines[end].paragraphEnd)
			end++;

		for (int i = start; i < _cursorRow; i++)
			ppos += getLineCharWidth(i);
		ppos += _cursorCol;

		Common::U32String pre_str = getTextChunk(start, 0, _cursorRow, _cursorCol, true, true);
		Common::U32String sub_str = getTextChunk(_cursorRow, _cursorCol, end, getLineCharWidth(end, true), true, true);

		// Remove it from the text
		for (int i = start; i <= end; i++) {
			_textLines.remove_at(start);
		}
		splitString(pre_str + str + sub_str, start);

		_cursorRow = start;
	}

	while (ppos > getLineCharWidth(_cursorRow, true)) {
		ppos -= getLineCharWidth(_cursorRow, true);
		_cursorRow++;
	}
	_cursorCol = ppos;
	recalcDims();
	updateCursorPos();
	_fullRefresh = true;
	render();
}

void MacText::setText(const Common::U32String &str) {
	_str = str;
	_textLines.clear();
	splitString(_str);

	_cursorRow = _cursorCol = 0;
	recalcDims();
	updateCursorPos();
	_fullRefresh = true;
	render();

	if (_selectable) {
		setSelection(_selStart, true);
		setSelection(_selEnd, false);
	}

	_contentIsDirty = true;
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

	Common::U32String newchunk = line->chunks[ch].text;

	if (pos >= (int)newchunk.size())
		newchunk += c;
	else
		newchunk.insertChar(c, pos);
	int chunkw = getStringWidth(line->chunks[ch], newchunk);
	int oldw = getStringWidth(line->chunks[ch], line->chunks[ch].text);

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
	for (int i = 0; i < (int)_textLines.size(); i++) {
		D(9, "**insertChar line %d isEnd %d", i, _textLines[i].paragraphEnd);
		for (int j = 0; j < (int)_textLines[i].chunks.size(); j++) {
			D(9, "[%d] \"%s\"",_textLines[i].chunks[j].text.size(), Common::toPrintable(_textLines[i].chunks[j].text.encode()).c_str());
		}
	}
	D(9, "**insertChar cursor row %d col %d", _cursorRow, _cursorCol);
}

void MacText::deleteSelection() {
	// TODO: maybe we need to implement an individual delete part for mactext
	if (_selectedText.endY == -1 || (_selectedText.startX == _selectedText.endX && _selectedText.startY == _selectedText.endY))
		return;
	if (_selectedText.startRow == -1 || _selectedText.startCol == -1 || _selectedText.endRow == -1 || _selectedText.endCol == -1)
		error("deleting non-existing selected area");

	SelectedText s = _selectedText;
	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startX, s.endX);
		SWAP(s.startY, s.endY);
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	int row = s.endRow, col = s.endCol;

	while (row != s.startRow || col != s.startCol) {
		if (row == 0 && col == 0)
			break;
		deletePreviousCharInternal(&row, &col);
	}

	reshuffleParagraph(&row, &col);

	_fullRefresh = true;
	recalcDims();
	render();

	// update cursor position
	_cursorRow = row;
	_cursorCol = col;
	updateCursorPos();
}

void MacText::deletePreviousCharInternal(int *row, int *col) {
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
		_textLines[*row].paragraphEnd = false;

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
}

void MacText::deletePreviousChar(int *row, int *col) {
	if (*col == 0 && *row == 0) // nothing to do
		return;
	deletePreviousCharInternal(row, col);

	for (int i = 0; i < (int)_textLines.size(); i++) {
		D(9, "**deleteChar line %d", i);
		for (int j = 0; j < (int)_textLines[i].chunks.size(); j++) {
			D(9, "[%d] \"%s\"",_textLines[i].chunks[j].text.size(), Common::toPrintable(_textLines[i].chunks[j].text.encode()).c_str());
		}
	}
	D(9, "**deleteChar cursor row %d col %d", _cursorRow, _cursorCol);

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

	// we have to inherit paragraphEnd from the origin line
	newline.paragraphEnd = line->paragraphEnd;

	newchunk.text = line->chunks[ch].text.substr(pos);
	line->chunks[ch].text = line->chunks[ch].text.substr(0, pos);
	line->paragraphEnd = true;
	newline.chunks.push_back(newchunk);

	for (uint i = ch + 1; i < line->chunks.size(); i++) {
		newline.chunks.push_back(MacFontRun(line->chunks[i]));
	}
	for (uint i = line->chunks.size() - 1; i >= ch + 1; i--) {
		line->chunks.pop_back();
	}
	line->width = -1; // Drop cache

	_textLines[*row].width = -1; // flush the cache

	_textLines.insert_at(*row + 1, newline);

	(*row)++;
	*col = 0;

	reshuffleParagraph(row, col);

	for (int i = 0; i < (int)_textLines.size(); i++) {
		D(9, "** addNewLine line %d", i);
		for (int j = 0; j < (int)_textLines[i].chunks.size(); j++) {
			D(9, "[%d] \"%s\"",_textLines[i].chunks[j].text.size(), Common::toPrintable(_textLines[i].chunks[j].text.encode()).c_str());
		}
	}
	D(9, "** addNewLine cursor row %d col %d", _cursorRow, _cursorCol);

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
	Common::U32String paragraph = getTextChunk(start, 0, end, getLineCharWidth(end, true), true, true);

	// Remove it from the text
	for (int i = start; i <= end; i++) {
		_textLines.remove_at(start);
	}

	// And now read it
	D(9, "start %d end %d", start, end);
	splitString(paragraph, start);

	// Find new pos within paragraph after reshuffling
	*row = start;

	while (ppos > getLineCharWidth(*row, true)) {
		ppos -= getLineCharWidth(*row, true);
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

		int alignOffset = getAlignOffset(_cursorRow);

		_cursorY = _textLines[_cursorRow].y - _scrollPos;
		_cursorX = getLineWidth(_cursorRow, false, _cursorCol) + alignOffset;
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
	_composeSurface->blitFrom(*_cursorSurface2, *_cursorRect, Common::Point(_cursorX + offset.x, _cursorY + offset.y));
}

} // End of namespace Graphics
