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

#include "common/file.h"
#include "common/timer.h"
#include "common/tokenizer.h"
#include "common/unicode-bidi.h"
#include "common/compression/unzip.h"

#include "graphics/font.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindow.h"

#ifdef USE_PNG
#include "image/png.h"
#endif

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
	if (wm->_mode & kWMModeWin95)
		return Common::kUtf8;
	return wm->_fontMan->getFontEncoding(fontId);
}

bool MacFontRun::plainByteMode() {
	Common::CodePage encoding = getEncoding();
	// This return statement accounts for utf8, invalid.
	// For future Unicode font compatibility, it should account for all codepages instead.
	return encoding != Common::kUtf8 && encoding != Common::kCodePageInvalid;
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
	_macFont(macFont) {

	D(6, "MacText::MacText(): fgcolor: %d, bgcolor: %d s: \"%s\"", fgcolor, bgcolor, Common::toPrintable(s.encode()).c_str());

	_str = s;
	_fullRefresh = true;

	_fixedDims = fixedDims;
	_wm = wm;

	_canvas._maxWidth = maxWidth;
	_canvas._textAlignment = textAlignment;
	_canvas._textShadow = textShadow;
	_canvas._interLinear = interlinear;
	_canvas._wm = wm;
	_canvas._fgcolor = fgcolor;
	_canvas._bgcolor = bgcolor;
	_canvas._macFontMode = true;
	_canvas._macText = this;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
		byte r, g, b;
		_wm->_pixelformat.colorToRGB(fgcolor, r, g, b);
		_defaultFormatting.setValues(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), r, g, b);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

// NOTE: This constructor and the one afterward are for MacText engines that don't use widgets. This is the classic was MacText was constructed.
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *macFont, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, bool fixedDims) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0),
	_macFont(macFont) {

	_str = s;

	_fixedDims = fixedDims;
	_wm = wm;

	_canvas._maxWidth = maxWidth;
	_canvas._textAlignment = textAlignment;
	_canvas._textShadow = 0;
	_canvas._interLinear = interlinear;
	_canvas._wm = wm;
	_canvas._fgcolor = fgcolor;
	_canvas._bgcolor = bgcolor;
	_canvas._macFontMode = true;
	_canvas._macText = this;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
		byte r, g, b;
		_wm->_pixelformat.colorToRGB(fgcolor, r, g, b);
		_defaultFormatting.setValues(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), r, g, b);
	} else {
		_defaultFormatting.font = NULL;
	}

	init();
}

// Working with plain Font
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const Font *font, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, bool fixedDims) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0),
	_macFont(nullptr) {

	_str = s;

	_fixedDims = fixedDims;
	_wm = wm;

	_canvas._maxWidth = maxWidth;
	_canvas._textAlignment = textAlignment;
	_canvas._textShadow = 0;
	_canvas._interLinear = interlinear;
	_canvas._wm = wm;
	_canvas._fgcolor = fgcolor;
	_canvas._bgcolor = bgcolor;
	_canvas._macFontMode = false;
	_canvas._macText = this;

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

	_canvas._textMaxWidth = 0;
	_canvas._textMaxHeight = 0;
	_canvas._surface = nullptr;
	_canvas._shadowSurface = nullptr;

	if (!_fixedDims) {
		int right = _dims.right;
		_dims.right = MAX<int>(_dims.right, _dims.left + _canvas._maxWidth + (2 * _border) + (2 * _gutter) + _shadow);
		if (right != _dims.right) {
			delete _composeSurface;
			_composeSurface = new ManagedSurface(_dims.width(), _dims.height(), _wm->_pixelformat);
		}
	}

	_selEnd = -1;
	_selStart = -1;

	_defaultFormatting.wm = _wm;

	_canvas.splitString(_str, -1, _defaultFormatting);
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
	_cursorCol = _canvas.getLineCharWidth(_cursorRow);

	_cursorRect = new Common::Rect(0, 0, 1, 1);

	// currently, we are not using fg color to render text. And we are not passing fg color correctly, thus we read it our self.
	MacFontRun colorFontRun = getFgColor();
	if (!colorFontRun.text.empty()) {
		_canvas._fgcolor = colorFontRun.fgcolor;
		colorFontRun.text.clear();
		debug(9, "Reading fg color though text, instead of the argument, read %d", _canvas._fgcolor);
		_defaultFormatting = colorFontRun;
		_defaultFormatting.wm = _wm;
	}

	_currentFormatting = _defaultFormatting;
	_composeSurface->clear(_bgcolor);

	_cursorSurface = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface->clear(_canvas._fgcolor);
	_cursorSurface2 = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface2->clear(_canvas._bgcolor);

	_canvas.reallocSurface();
	setAlignOffset(_canvas._textAlignment);
	updateCursorPos();
	render();
}

MacText::~MacText() {
	if (_wm->getActiveWidget() == this)
		_wm->setActiveWidget(nullptr);

	delete _cursorRect;
	delete _cursorSurface;
	delete _cursorSurface2;

#ifdef USE_PNG
	for (auto &i : _imageCache)
		delete i._value;
#endif

	delete _imageArchive;
}

// this func returns the fg color of the first character we met in text
MacFontRun MacText::getFgColor() {
	if (_canvas._text.empty())
		return MacFontRun();
	for (uint i = 0; i < _canvas._text.size(); i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			if (!_canvas._text[i].chunks[j].text.empty())
				return _canvas._text[i].chunks[j];
		}
	}
	return MacFontRun();
}

// we are doing this because we may need to dealing with the plain byte. See ctor of mactext which contains String str instead of U32String str
// thus, if we are passing the str, meaning we are using plainByteMode. And when we calculate the string width. we need to convert it to it's original state first;
int getStringWidth(MacFontRun &format, const Common::U32String &str) {
	if (format.plainByteMode())
		return format.getFont()->getStringWidth(Common::convertFromU32String(str, format.getEncoding()));
	else
		return format.getFont()->getStringWidth(str);
}

int getStringMaxWordWidth(MacFontRun &format, const Common::U32String &str) {
	if (format.plainByteMode()) {
		Common::StringTokenizer tok(Common::convertFromU32String(str, format.getEncoding()));
		int maxW = 0;

		while (!tok.empty()) {
			int w = format.getFont()->getStringWidth(tok.nextToken());

			maxW = MAX(maxW, w);
		}

		return maxW;
	} else {
		Common::U32StringTokenizer tok(str);
		int maxW = 0;

		while (!tok.empty()) {
			int w = format.getFont()->getStringWidth(tok.nextToken());

			maxW = MAX(maxW, w);
		}

		return maxW;
	}
}


void MacText::setMaxWidth(int maxWidth) {
	if (maxWidth == _canvas._maxWidth)
		return;

	if (maxWidth < 0) {
		warning("trying to set maxWidth to %d", maxWidth);
		return;
	}

	Common::U32String str = getTextChunk(0, 0, -1, -1, true, true);

	// keep the cursor pos
	int ppos = 0;
	for (int i = 0; i < _cursorRow; i++)
		ppos += _canvas.getLineCharWidth(i);
	ppos += _cursorCol;

	_canvas._maxWidth = maxWidth;
	_canvas._text.clear();

	_canvas.splitString(str, -1, _defaultFormatting);

	// restore the cursor pos
	_cursorRow = 0;
	warning("FIXME, bad design");
	while (ppos > _canvas.getLineCharWidth(_cursorRow, true)) {
		ppos -= _canvas.getLineCharWidth(_cursorRow, true);
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
	_canvas._bgcolor = bg;
	_canvas._fgcolor = fg;
	// also set the cursor color
	_cursorSurface->clear(_canvas._fgcolor);
	for (uint i = 0; i < _canvas._text.size(); i++)
		setTextColor(fg, i);

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::enforceTextFont(uint16 fontId) {
	for (uint i = 0; i < _canvas._text.size(); i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			_canvas._text[i].chunks[j].fontId = fontId;
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::setTextSize(int textSize) {
	for (uint i = 0; i < _canvas._text.size(); i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			_canvas._text[i].chunks[j].fontSize = textSize;
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::setTextColor(uint32 color, uint32 line) {
	if (line >= _canvas._text.size()) {
		warning("MacText::setTextColor(): line %d is out of bounds", line);
		return;
	}

	uint32 fgcol = _wm->findBestColor(color);
	for (uint j = 0; j < _canvas._text[line].chunks.size(); j++) {
		_canvas._text[line].chunks[j].fgcolor = fgcol;
	}

	// if we are calling this func separately, then here need a refresh
}

void MacText::getChunkPosFromIndex(int index, uint &lineNum, uint &chunkNum, uint &offset) {
	if (_canvas._text.empty()) {
		lineNum = chunkNum = offset = 0;
		return;
	}
	for (uint i = 0; i < _canvas._text.size(); i++) {
		if (_canvas.getLineCharWidth(i) <= index) {
			index -= _canvas.getLineCharWidth(i);
		} else {
			lineNum = i;
			chunkNum = _canvas._text[i].getChunkNum(&index);
			offset = index;
			return;
		}
	}
	lineNum = _canvas._text.size() - 1;
	chunkNum = _canvas._text[lineNum].chunks.size() - 1;
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
	if (_canvas._text.empty())
		return;
	if (start > end)
		SWAP(start, end);

	uint startRow, startCol;
	uint endRow, endCol;
	uint offset;

	getChunkPosFromIndex(start, startRow, startCol, offset);
	// if offset != 0, then we need to split the chunk
	if (offset != 0) {
		uint textSize = _canvas._text[startRow].chunks[startCol].text.size();
		MacFontRun newChunk = _canvas._text[startRow].chunks[startCol];
		newChunk.text = newChunk.text.substr(offset, textSize - offset);
		_canvas._text[startRow].chunks[startCol].text = _canvas._text[startRow].chunks[startCol].text.substr(0, offset);
		_canvas._text[startRow].chunks.insert_at(startCol + 1, newChunk);
		startCol++;
	}

	getChunkPosFromIndex(end, endRow, endCol, offset);
	if (offset != 0) {
		uint textSize = _canvas._text[endRow].chunks[endCol].text.size();
		MacFontRun newChunk = _canvas._text[endRow].chunks[endCol];
		newChunk.text = newChunk.text.substr(offset, textSize - offset);
		_canvas._text[endRow].chunks[endCol].text = _canvas._text[endRow].chunks[endCol].text.substr(0, offset);
		_canvas._text[endRow].chunks.insert_at(endCol + 1, newChunk);
		endCol++;
	}

	for (uint i = startRow; i <= endRow; i++) {
		uint from, to;
		if (i == startRow && i == endRow) {
			from = startCol;
			to = endCol;
		} else if (i == startRow) {
			from = startCol;
			to = _canvas._text[startRow].chunks.size();
		} else if (i == endRow) {
			from = 0;
			to = endCol;
		} else {
			from = 0;
			to = _canvas._text[i].chunks.size();
		}
		for (uint j = from; j < to; j++) {
			callback(_canvas._text[i].chunks[j], param);
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
	for (uint i = 0; i < _canvas._text.size(); i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			if (textSlant) {
				_canvas._text[i].chunks[j].textSlant |= textSlant;
			} else {
				_canvas._text[i].chunks[j].textSlant = textSlant;
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
	if (_canvas._text.empty())
		return _defaultFormatting;
	if (start > end)
		SWAP(start, end);

	uint startRow, startCol;
	uint offset;

	getChunkPosFromIndex(start, startRow, startCol, offset);
	return _canvas._text[startRow].chunks[startCol];
}

void MacText::setDefaultFormatting(uint16 fontId, byte textSlant, uint16 fontSize,
		uint16 palinfo1, uint16 palinfo2, uint16 palinfo3) {
	_defaultFormatting.setValues(_defaultFormatting.wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

	MacFont macFont = MacFont(fontId, fontSize, textSlant);

	_defaultFormatting.font = _wm->_fontMan->getFont(macFont);
}

// Adds the given string to the end of the last line/chunk
// while observing the _canvas._maxWidth and keeping this chunk's
// formatting
void MacTextCanvas::chopChunk(const Common::U32String &str, int *curLinePtr, int indent, int maxWidth) {
	int curLine = *curLinePtr;
	int curChunk;
	MacFontRun *chunk;

	curChunk = _text[curLine].chunks.size() - 1;
	chunk = &_text[curLine].chunks[curChunk];

	// Check if there is nothing to add, then remove the last chunk
	// This happens when the previous run is finished only with
	// empty formatting, or when we were adding text for the first time
	if (chunk->text.empty() && str.empty()) {
		D(9, "** chopChunk, replaced formatting, line %d", curLine);

		_text[curLine].chunks.pop_back();

		return;
	}

	if (maxWidth == -1) {
		chunk->text += str;

		return;
	}

	Common::Array<Common::U32String> text;

	int w = getLineWidth(curLine, true);
	D(9, "** chopChunk before wrap \"%s\"", Common::toPrintable(str.encode()).c_str());

	chunk->getFont()->wordWrapText(str, maxWidth, text, w);

	if (text.size() == 0) {
		warning("chopChunk: too narrow width, >%d", maxWidth);
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

	D(9, "** chopChunk, subchunk: \"%s\" (%d lines, maxW: %d)", toPrintable(text[0].encode()).c_str(), text.size(), maxWidth);

	// We do not overlap, so we're done
	if (text.size() == 1)
		return;

	// Now add rest of the chunks
	MacFontRun newchunk = _text[curLine].chunks[curChunk];

	for (uint i = 1; i < text.size(); i++) {
		newchunk.text = text[i];

		curLine++;
		_text.insert_at(curLine, MacTextLine());
		_text[curLine].chunks.push_back(newchunk);
		_text[curLine].indent = indent;
		_text[curLine].firstLineIndent = 0;

		D(9, "** chopChunk, added line (firstIndent: %d): \"%s\"", _text[curLine].firstLineIndent, toPrintable(text[i].encode()).c_str());
	}

	*curLinePtr = curLine;
}

void MacTextCanvas::splitString(const Common::U32String &str, int curLine, MacFontRun &defaultFormatting) {
	D(9, "** splitString(\"%s\", %d)", toPrintable(str.encode()).c_str(), curLine);

	if (str.empty()) {
		D(9, "** splitString, empty line");
		return;
	}

	(void)splitString(str.c_str(), curLine, defaultFormatting);
}

const Common::U32String::value_type *MacTextCanvas::splitString(const Common::U32String::value_type *s, int curLine, MacFontRun &defaultFormatting) {
	if (_text.empty()) {
		_text.resize(1);
		_text[0].chunks.push_back(defaultFormatting);
		D(9, "** splitString, added default formatting");
	} else {
		D(9, "** splitString, continuing, %d lines", _text.size());
	}

	Common::U32String tmp;

	if (curLine == -1 || curLine >= (int)_text.size())
		curLine = _text.size() - 1;

	int curChunk = _text[curLine].chunks.size() - 1;
	MacFontRun chunk = _text[curLine].chunks[curChunk];
	int indentSize = 0;
	int firstLineIndent = 0;

	while (*s) {
		firstLineIndent = 0;

		tmp.clear();

		MacTextLine *curTextLine = &_text[curLine];

		while (*s) {
			bool endOfLine = false;

			// Scan till next font change or end of line
			while (*s && *s != '\001') {
				if (*s == '\r') {
					s++;
					if (*s == '\n')	// Skip whole '\r\n'
						s++;

					endOfLine = true;

					break;
				}

				// deal with single \n
				if (*s == '\n') {
					s++;

					endOfLine = true;
					break;
				}

				tmp += *s;

				s++;
			}

			// If we reached end of paragraph, go to outer loop
			if (endOfLine)
				break;

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
			chopChunk(tmp, &curLine, indentSize, _maxWidth > 0 ? _maxWidth - indentSize : _maxWidth);

			curTextLine = &_text[curLine];

			firstLineIndent = curTextLine->firstLineIndent;

			tmp.clear();

			// If it is end of the line, we're done
			if (!*s) {
				D(9, "** splitString, end of line");

				break;
			}

			// get format (sync with stripFormat() )
			if (*s == '\016') {	// human-readable format
				s++;

				// First two digits is slant, third digit is Header number
				switch (*s) {
				case '+': { // \016+XXYZ  -- opening textSlant, H<Y>, indent<+Z>
					uint16 textSlant, headSize, indent;
					s++;

					s = readHex(&textSlant, s, 2);

					chunk.textSlant |= textSlant; // Setting the specified bit

					s = readHex(&headSize, s, 1);
					if (headSize >= 1 && headSize <= 6) { // set
						const float sizes[] = { 1, 2.0f, 1.41f, 1.155f, 1.0f, .894f, .816f };
						chunk.fontSize = defaultFormatting.fontSize * sizes[headSize];
					}

					s = readHex(&indent, s, 1);

					if (s)
						indentSize += indent * chunk.fontSize * 2;

					D(9, "** splitString+: fontId: %d, textSlant: %d, fontSize: %d, indent: %d",
							chunk.fontId, chunk.textSlant, chunk.fontSize,
							indent);

					break;
					}
				case '-': { // \016-XXYZ  -- closing textSlant, H<Y>, indent<+Z>
					uint16 textSlant, headSize, indent;
					s++;

					s = readHex(&textSlant, s, 2);

					chunk.textSlant &= ~textSlant; // Clearing the specified bit

					s = readHex(&headSize, s, 1);
					if (headSize == 0xf) // reset
						chunk.fontSize = defaultFormatting.fontSize;

					s = readHex(&indent, s, 1);

					if (s)
						indentSize -= indent * chunk.fontSize * 2;

					D(9, "** splitString-: fontId: %d, textSlant: %d, fontSize: %d, indent: %d",
							chunk.fontId, chunk.textSlant, chunk.fontSize,
							indent);
					break;
					}

				case '[': { // \016[RRGGBB  -- setting color
					uint16 palinfo1, palinfo2, palinfo3;
					s++;

					s = readHex(&palinfo1, s, 4);
					s = readHex(&palinfo2, s, 4);
					s = readHex(&palinfo3, s, 4);

					chunk.palinfo1 = palinfo1;
					chunk.palinfo2 = palinfo2;
					chunk.palinfo3 = palinfo3;
					chunk.fgcolor  = _wm->findBestColor(palinfo1 & 0xff, palinfo2 & 0xff, palinfo3 & 0xff);

					D(9, "** splitString[: %08x", chunk.fgcolor);
					break;
					}

				case ']': { // \016]  -- setting default color
					s++;

					chunk.palinfo1 = defaultFormatting.palinfo1;
					chunk.palinfo2 = defaultFormatting.palinfo2;
					chunk.palinfo3 = defaultFormatting.palinfo3;
					chunk.fgcolor  = defaultFormatting.fgcolor;

					D(9, "** splitString]: %08x", chunk.fgcolor);
					break;
					}

				case '*': { // \016*XXsssssss  -- negative indent, XX size, sssss is the string
					s++;

					uint16 len;

					s = readHex(&len, s, 2);

					Common::U32String bullet = Common::U32String(s, len);

					s += len;

					firstLineIndent = -chunk.getFont()->getStringWidth(bullet);

					D(9, "** splitString*: %02x '%s' (%d)", len, bullet.encode().c_str(), firstLineIndent);
					break;
					}

				case 'i': { // \016iXXNNnnnnAAaaaaTTttt -- image, XX% width,
										//          NN, nnnn -- filename len and text
										//          AA, aaaa -- alt len and text
										//          TT, tttt -- text (tooltip) len and text
					s++;

					uint16 len;

					s = readHex(&_text[curLine].picpercent, s, 2);
					s = readHex(&len, s, 2);
					_text[curLine].picfname = Common::U32String(s, len).encode();
					s += len;

					s = readHex(&len, s, 2);
					_text[curLine].picalt = Common::U32String(s, len);
					s += len;

					s = readHex(&len, s, 2);
					_text[curLine].pictitle = Common::U32String(s, len);
					s += len;

					D(9, "** splitString[i]: %d%% fname: '%s'  alt: '%s'  title: '%s'",
						_text[curLine].picpercent,
						_text[curLine].picfname.c_str(), _text[curLine].picalt.encode().c_str(),
						_text[curLine].pictitle.encode().c_str());
					break;
					}

				case 't': { // \016tXXXX -- switch to the requested font id
					s++;

					uint16 fontId;

					s = readHex(&fontId, s, 4);

					chunk.fontId = fontId == 0xffff ? defaultFormatting.fontId : fontId;

					D(9, "** splitString[t]: fontId: %d", fontId);
					break;
					}

				case 'l': { // \016lLLllll -- link len and text
					s++;

					uint16 len;

					s = readHex(&len, s, 2);
					chunk.link = Common::U32String(s, len);
					s += len;

					D(9, "** splitString[l]: link: %s", chunk.link.c_str());
					break;
					}

				case 'T': { // \016T -- table
					s++;

					char cmd = *s++;

					if (cmd == 'h') { // Header, beginning of the table
						curTextLine->table = new Common::Array<MacTextTableRow>();

						D(9, "** splitString[table header]");
					} else if (cmd == 'b') { // Body start
						D(9, "** splitString[body start]");
					} else if (cmd == 'B') { // Body end
						D(9, "** splitString[body end]");
						processTable(curLine, _maxWidth);

						continue;
					} else if (cmd == 'r') { // Row
						curTextLine->table->push_back(MacTextTableRow());
						D(9, "** splitString[row]");
					} else if (cmd == 'c') { // Cell start
						uint16 align;
						s = readHex(&align, s, 2);

						curTextLine->table->back().cells.push_back(MacTextCanvas());

						MacTextCanvas *cellCanvas = &curTextLine->table->back().cells.back();
						cellCanvas->_textAlignment = (TextAlign)align;
						cellCanvas->_wm = _wm;
						cellCanvas->_macText = _macText;
						cellCanvas->_maxWidth = -1;
						cellCanvas->_macFontMode = _macFontMode;
						cellCanvas->_fgcolor = _fgcolor;
						cellCanvas->_bgcolor = _bgcolor;

						D(9, "** splitString[cell start]: align: %d", align);

						D(9, "** splitString[RECURSION start]");

						s = cellCanvas->splitString(s, curLine, defaultFormatting);

						D(9, "** splitString[RECURSION end]");
					} else if (cmd == 'C') { // Cell end
						D(9, "** splitString[cell end]");

						return s;
					} else {
						error("MacText: Unknown table subcommand (%c)", cmd);
					}
					break;
					}

				default: {
					uint16 fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3;

					s = readHex(&fontId, s, 4);
					s = readHex(&textSlant, s, 2);
					s = readHex(&fontSize, s, 4);
					s = readHex(&palinfo1, s, 4);
					s = readHex(&palinfo2, s, 4);
					s = readHex(&palinfo3, s, 4);

					chunk.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

					D(9, "** splitString: fontId: %d, textSlant: %d, fontSize: %d, fg: %04x",
							fontId, textSlant, fontSize, chunk.fgcolor);

					// So far, we enforce single font here, though in the future, font size could be altered
					if (!_macFontMode)
						chunk.font = defaultFormatting.font;
					}
				}
			}


			curTextLine->indent = indentSize;
			curTextLine->firstLineIndent = firstLineIndent;

			// Push new formatting
			curTextLine->chunks.push_back(chunk);
		}

		if (!*s) { // If this is end of the string, we're done here
			break;
		}

		// Add new line
		D(9, "** splitString: new line");

		curTextLine->paragraphEnd = true;
		// if the chunks is empty, which means the line will not be rendered properly
		// so we add a empty string here
		if (curTextLine->chunks.empty()) {
			curTextLine->chunks.push_back(defaultFormatting);
		}

		curLine++;
		_text.insert_at(curLine, MacTextLine());
		_text[curLine].chunks.push_back(chunk);

		curTextLine = &_text[curLine];
	}

#if DEBUG
	for (uint i = 0; i < _text.size(); i++) {
		debugN(9, "** splitString: %2d ", i);

		for (uint j = 0; j < _text[i].chunks.size(); j++)
			debugN(9, "[%d] \"%s\"", _text[i].chunks[j].text.size(), Common::toPrintable(_text[i].chunks[j].text.encode()).c_str());

		debugN(9, "\n");
	}
	debug(9, "** splitString: done");
#endif

	return s;
}


void MacTextCanvas::reallocSurface() {
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
		_canvas._surface->clear(_bgcolor);
		if (_canvas._textShadow)
			_canvas._shadowSurface->clear(_bgcolor);

		_canvas.render(0, _canvas._text.size());

		_fullRefresh = false;

#if 0
		Common::DumpFile out;
		Common::String filename = Common::String::format("z-%p.png", (void *)this);
		if (out.open(filename)) {
			warning("Wrote: %s", filename.c_str());
			Image::writePNG(out, _canvas._surface->rawSurface());
		}
#endif
	}
}

void MacTextCanvas::render(int from, int to, int shadow) {
	int w = MIN(_maxWidth, _textMaxWidth);
	ManagedSurface *surface = shadow ? _shadowSurface : _surface;

	int myFrom = from, myTo = to + 1, delta = 1;

	if (_wm->_language == Common::HE_ISR) {
		myFrom = to;
		myTo = from - 1;
		delta = -1;
	}

	for (int i = myFrom; i != myTo; i += delta) {
		if (!_text[i].picfname.empty()) {
			const Surface *image = _macText->getImageSurface(_text[i].picfname);

			int xOffset = (_text[i].width - _text[i].charwidth) / 2;
			Common::Rect bbox(xOffset, _text[i].y, xOffset + _text[i].charwidth, _text[i].y + _text[i].height);

			if (image)
				surface->blitFrom(image, Common::Rect(0, 0, image->w, image->h), bbox);

			continue;
		}

		if (_text[i].tableSurface) {
			surface->blitFrom(*_text[i].tableSurface, Common::Point(0, _text[i].y));

			continue;
		}

		int xOffset = getAlignOffset(i) + _text[i].indent + _text[i].firstLineIndent;
		xOffset++;

		int start = 0, end = _text[i].chunks.size();
		if (_wm->_language == Common::HE_ISR) {
			start = _text[i].chunks.size() - 1;
			end = -1;
		}

		int maxAscentForRow = 0;
		for (int j = start; j != end; j += delta) {
			if (_text[i].chunks[j].font->getFontAscent() > maxAscentForRow)
				maxAscentForRow = _text[i].chunks[j].font->getFontAscent();
		}

		// TODO: _canvas._textMaxWidth, when -1, was not rendering ANY text.
		for (int j = start; j != end; j += delta) {
			D(9, "MacTextCanvas::render: line %d[%d] h:%d at %d,%d (%s) fontid: %d fontsize: %d on %dx%d, fgcolor: %08x bgcolor: %08x, font: %p",
				  i, j, _text[i].height, xOffset, _text[i].y, _text[i].chunks[j].text.encode().c_str(),
				  _text[i].chunks[j].fontId, _text[i].chunks[j].fontSize, _surface->w, _surface->h, _text[i].chunks[j].fgcolor, _bgcolor,
				  (const void *)_text[i].chunks[j].getFont());

			if (_text[i].chunks[j].text.empty())
				continue;

			int yOffset = 0;
			if (_text[i].chunks[j].font->getFontAscent() < maxAscentForRow) {
				yOffset = maxAscentForRow - _text[i].chunks[j].font->getFontAscent();
			}

			if (_text[i].chunks[j].plainByteMode()) {
				Common::String str = _text[i].chunks[j].getEncodedText();
				_text[i].chunks[j].getFont()->drawString(surface, str, xOffset, _text[i].y + yOffset, w, shadow ? _wm->_colorBlack : _text[i].chunks[j].fgcolor, kTextAlignLeft, 0, true);
				xOffset += _text[i].chunks[j].getFont()->getStringWidth(str);
			} else {
				if (_wm->_language == Common::HE_ISR)
					_text[i].chunks[j].getFont()->drawString(surface, convertBiDiU32String(_text[i].chunks[j].text, Common::BIDI_PAR_RTL), xOffset, _text[i].y + yOffset, w, shadow ? _wm->_colorBlack : _text[i].chunks[j].fgcolor, kTextAlignLeft, 0, true);
				else
					_text[i].chunks[j].getFont()->drawString(surface, convertBiDiU32String(_text[i].chunks[j].text), xOffset, _text[i].y + yOffset, w, shadow ? _wm->_colorBlack : _text[i].chunks[j].fgcolor, kTextAlignLeft, 0, true);
				xOffset += _text[i].chunks[j].getFont()->getStringWidth(_text[i].chunks[j].text);
			}
		}
	}
}

void MacTextCanvas::render(int from, int to) {
	if (_text.empty())
		return;

	reallocSurface();

	from = MAX<int>(0, from);
	to = MIN<int>(to, _text.size() - 1);

	// Clear the screen
	_surface->fillRect(Common::Rect(0, _text[from].y, _surface->w, _text[to].y + getLineHeight(to)), _bgcolor);

	// render the shadow surface;
	if (_textShadow)
		render(from, to, _textShadow);

	render(from, to, 0);

	for (uint i = 0; i < _text.size(); i++) {
		debugN(9, "MacTextCanvas::render: %2d (firstInd: %d indent: %d) ", i, _text[i].firstLineIndent, _text[i].indent);

		for (uint j = 0; j < _text[i].chunks.size(); j++)
			debugN(9, "[%d (%d)] \"%s\" ", _text[i].chunks[j].fontId, _text[i].chunks[j].textSlant, _text[i].chunks[j].text.encode().c_str());

		debug(9, "%s", "");
	}
}

int MacTextCanvas::getLineWidth(int lineNum, bool enforce, int col) {
	if ((uint)lineNum >= _text.size())
		return 0;

	MacTextLine *line = &_text[lineNum];

	if (line->width != -1 && !enforce && col == -1)
		return line->width;

	if (!line->picfname.empty()) {
		const Surface *image = _macText->getImageSurface(line->picfname);

		if (image) {
			float ratio = _maxWidth * line->picpercent / 100.0 / (float)image->w;
			line->width = _maxWidth;
			line->height = image->h * ratio;
			line->charwidth = image->w * ratio;
		} else {
			line->width = _maxWidth;
			line->height = 1;
			line->charwidth = 1;
		}

		return line->width;
	}

	if (line->table) {
		line->width = _maxWidth;
		line->height = line->tableSurface->h;
		line->charwidth = _maxWidth;

		return line->width;
	}

	int width = line->indent + line->firstLineIndent;
	int height = 0;
	int charwidth = 0;
	int minWidth = 0;
	bool firstWord = true;

	for (uint i = 0; i < line->chunks.size(); i++) {
		if (enforce && _macFontMode)
			line->chunks[i].font = nullptr;

		if (col >= 0) {
			if (col >= (int)line->chunks[i].text.size()) {
				col -= line->chunks[i].text.size();
			} else {
				Common::U32String tmp = line->chunks[i].text.substr(0, col);

				width += getStringWidth(line->chunks[i], tmp);

				return width;
			}
		}

		if (!line->chunks[i].text.empty()) {
			int w = getStringWidth(line->chunks[i], line->chunks[i].text);
			int mW = getStringMaxWordWidth(line->chunks[i], line->chunks[i].text);

			if (firstWord) {
				minWidth = mW + width; // Take indent into account
				firstWord = false;
			} else {
				minWidth = MAX(minWidth, mW);
			}
			width += w;
			charwidth += line->chunks[i].text.size();
		}

		height = MAX(height, line->chunks[i].getFont()->getFontHeight());
	}


	line->width = width;
	line->minWidth = minWidth;
	line->height = height;
	line->charwidth = charwidth;

	return width;
}

int MacTextCanvas::getLineCharWidth(int line, bool enforce) {
	if ((uint)line >= _text.size())
		return 0;

	if (_text[line].charwidth != -1 && !enforce)
		return _text[line].charwidth;

	int width = 0;

	for (uint i = 0; i < _text[line].chunks.size(); i++) {
		if (!_text[line].chunks[i].text.empty())
			width += _text[line].chunks[i].text.size();
	}

	_text[line].charwidth = width;

	return width;
}

int MacText::getLastLineWidth() {
	if (_canvas._text.size() == 0)
		return 0;

	return _canvas.getLineWidth(_canvas._text.size() - 1, true);
}

int MacText::getLineHeight(int line) {
	return _canvas.getLineHeight(line);
}

int MacTextCanvas::getLineHeight(int line) {
	if ((uint)line >= _text.size())
		return 0;

	(void)getLineWidth(line); // This calculates height also

	return _text[line].height;
}

void MacText::setInterLinear(int interLinear) {
	_canvas._interLinear = interLinear;

	recalcDims();
	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

void MacText::recalcDims() {
	_canvas.recalcDims();

	if (!_fixedDims) {
		int newBottom = _dims.top + _canvas._textMaxHeight + (2 * _border) + _gutter + _shadow;
		if (newBottom > _dims.bottom) {
			_dims.bottom = newBottom;
			delete _composeSurface;
			_composeSurface = new ManagedSurface(_dims.width(), _dims.height(), _wm->_pixelformat);
			_canvas.reallocSurface();
			if (!_fullRefresh) {
				_fullRefresh = true;
				render();
			}
			_fullRefresh = true;
			_contentIsDirty = true;
		}
	}
}

void MacTextCanvas::recalcDims() {
	if (_text.empty())
		return;

	int y = 0;
	_textMaxWidth = 0;

	for (uint i = 0; i < _text.size(); i++) {
		_text[i].y = y;

		// We must calculate width first, because it enforces
		// the computation. Calling Height() will return cached value!
		_textMaxWidth = MAX(_textMaxWidth, getLineWidth(i, true));
		y += MAX(getLineHeight(i), _interLinear);
	}

	_textMaxHeight = y;
}

void MacText::setAlignOffset(TextAlign align) {
	if (_canvas._textAlignment == align)
		return;

	_contentIsDirty = true;
	_fullRefresh = true;
	_canvas._textAlignment = align;
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
	if (_canvas._surface->w == w && _canvas._surface->h == h)
		return;

	setMaxWidth(w);
	if (_composeSurface->w != w || _composeSurface->h != h) {
		delete _composeSurface;
		_composeSurface = new ManagedSurface(w, h, _wm->_pixelformat);
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
	uint oldLen = _canvas._text.size();

	MacFontRun fontRun = MacFontRun(_wm, fontId, fontSlant, fontSize, r, g, b);

	_currentFormatting = fontRun;

	// we check _str here, if _str is empty but _canvas._text is not empty, and they are not the end of paragraph
	// then we remove those empty lines
	// too many special check may cause some strange problem in the future
	if (_str.empty()) {
		while (!_canvas._text.empty() && !_canvas._text.back().paragraphEnd)
			removeLastLine();
	}

	// we need to split the string with the font, in order to get the correct font
	Common::U32String strWithFont = Common::U32String(fontRun.toString()) + str;

	if (!skipAdd)
		_str += strWithFont;

	appendText_(strWithFont, oldLen);
}

void MacText::appendText(const Common::U32String &str, const Font *font, uint16 r, uint16 g, uint16 b, bool skipAdd) {
	uint oldLen = _canvas._text.size();

	MacFontRun fontRun = MacFontRun(_wm, font, 0, font->getFontHeight(), r, g, b);

	_currentFormatting = fontRun;

	if (_str.empty()) {
		while (!_canvas._text.empty() && !_canvas._text.back().paragraphEnd)
			removeLastLine();
	}

	Common::U32String strWithFont = Common::U32String(fontRun.toString()) + str;

	if (!skipAdd)
		_str += strWithFont;

	appendText_(strWithFont, oldLen);
}

void MacText::appendText_(const Common::U32String &strWithFont, uint oldLen) {
	_canvas.splitString(strWithFont, -1, _defaultFormatting);
	recalcDims();

	_canvas.render(oldLen - 1, _canvas._text.size());

	_contentIsDirty = true;

	if (_editable) {
		_scrollPos = MAX<int>(0, getTextHeight() - getDimensions().height());

		_cursorRow = getLineCount();
		_cursorCol = _canvas.getLineCharWidth(_cursorRow);

		updateCursorPos();
	}
}

void MacText::appendTextDefault(const Common::U32String &str, bool skipAdd) {
	uint oldLen = _canvas._text.size();

	_currentFormatting = _defaultFormatting;
	Common::U32String strWithFont = Common::U32String(_defaultFormatting.toString()) + str;

	if (!skipAdd) {
		_str += strWithFont;
	}
	_canvas.splitString(strWithFont, -1, _defaultFormatting);
	recalcDims();

	_canvas.render(oldLen - 1, _canvas._text.size());
}

void MacText::appendTextDefault(const Common::String &str, bool skipAdd) {
	appendTextDefault(Common::U32String(str), skipAdd);
}

void MacText::clearText() {
	_contentIsDirty = true;
	_canvas._text.clear();
	_str.clear();

	if (_canvas._surface)
		_canvas._surface->clear(_bgcolor);

	recalcDims();

	_cursorRow = _cursorCol = 0;
	updateCursorPos();
}

void MacText::removeLastLine() {
	if (!_canvas._text.size())
		return;

	int h = getLineHeight(_canvas._text.size() - 1) + _canvas._interLinear;

	_canvas._surface->fillRect(Common::Rect(0, _canvas._textMaxHeight - h, _canvas._surface->w, _canvas._textMaxHeight), _bgcolor);

	_canvas._text.pop_back();
	_canvas._textMaxHeight -= h;
}

void MacText::draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff) {
	if (_canvas._text.empty())
		return;

	render();

	if (x + w < _canvas._surface->w || y + h < _canvas._surface->h)
		g->fillRect(Common::Rect(x + xoff, y + yoff, x + w + xoff, y + h + yoff), _bgcolor);

	// blit shadow surface first
	if (_canvas._textShadow)
		g->blitFrom(*_canvas._shadowSurface, Common::Rect(MIN<int>(_canvas._surface->w, x), MIN<int>(_canvas._surface->h, y), MIN<int>(_canvas._surface->w, x + w), MIN<int>(_canvas._surface->h, y + h)), Common::Point(xoff + _canvas._textShadow, yoff + _canvas._textShadow));

	uint32 bgcolor = _bgcolor < 0xff ? _bgcolor : 0;
	g->transBlitFrom(*_canvas._surface, Common::Rect(MIN<int>(_canvas._surface->w, x), MIN<int>(_canvas._surface->h, y), MIN<int>(_canvas._surface->w, x + w), MIN<int>(_canvas._surface->h, y + h)), Common::Point(xoff, yoff), bgcolor);

	_contentIsDirty = false;
	_cursorDirty = false;
}

bool MacText::draw(bool forceRedraw) {
	if (!needsRedraw() && !forceRedraw)
		return false;

	if (!_canvas._surface) {
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

	draw(_composeSurface, 0, _scrollPos, _canvas._surface->w, _scrollPos + _canvas._surface->h, offset.x, offset.y);

	for (int bb = 0; bb < _shadow; bb++) {
		_composeSurface->hLine(_shadow, _composeSurface->h - _shadow + bb, _composeSurface->w, 0);
		_composeSurface->vLine(_composeSurface->w - _shadow + bb, _shadow, _composeSurface->h - _shadow, 0);
	}

	for (int bb = 0; bb < _border; bb++) {
		Common::Rect borderRect(bb, bb, _composeSurface->w - bb, _composeSurface->h - bb);
		_composeSurface->frameRect(borderRect, 0xff);
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
	if (_canvas._text.empty())
		return;

	render();

	srcRect.clip(_canvas._surface->getBounds());

	if (srcRect.isEmpty())
		return;

	g->blitFrom(*_canvas._surface, srcRect, dstPoint);
}

void MacText::drawToPoint(ManagedSurface *g, Common::Point dstPoint) {
	if (_canvas._text.empty())
		return;

	render();

	g->blitFrom(*_canvas._surface, dstPoint);
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

	if (s.endCol == _canvas.getLineCharWidth(s.endRow))
		s.endX = maxSelectionWidth;

	end = MIN((int)maxSelectionHeight, end);

	// if we are selecting all text, then we invert the whole area
	if ((uint)s.endRow == _canvas._text.size() - 1)
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
			alignOffset = _canvas.getAlignOffset(start_row);

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
		if (!numLines && (uint)row < _canvas._text.size()) {
			x1 = 0;
			x2 = maxSelectionWidth;

			alignOffset = _canvas.getAlignOffset(row);

			numLines = getLineHeight(row);
			if (y + _scrollPos == s.startY && s.startX > 0)
				x1 = s.startX;
			if (y + _scrollPos >= lastLineStart)
				x2 = s.endX;

			// if we are selecting text reversely, and we are at the first line but not the select from beginning, then we add offset to x1
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
			if (*ptr == _canvas._fgcolor)
				*ptr = _canvas._bgcolor;
			else
				*ptr = _canvas._fgcolor;
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
			pos += _canvas.getLineCharWidth(row);

		pos += _selectedText.startCol;
		return pos;
	} else {
		for (int row = 0; row < _selectedText.endRow; row++)
			pos += _canvas.getLineCharWidth(row);

		pos += _selectedText.endCol;
		return pos;
	}
}

void MacText::setSelection(int pos, bool start) {
	// -1 for start represent the beginning of text, i.e. 0
	if (pos == -1 && start)
		pos = 0;
	int row = 0, col = 0;
	int colX = 0;

	if (pos > 0) {
		while (pos > 0) {
			if (pos < _canvas.getLineCharWidth(row)) {
				for (uint i = 0; i < _canvas._text[row].chunks.size(); i++) {
					if ((uint)pos < _canvas._text[row].chunks[i].text.size()) {
						colX += getStringWidth(_canvas._text[row].chunks[i], _canvas._text[row].chunks[i].text.substr(0, pos));
						col += pos;
						pos = 0;
						break;
					} else {
						colX += getStringWidth(_canvas._text[row].chunks[i], _canvas._text[row].chunks[i].text);
						pos -= _canvas._text[row].chunks[i].text.size();
						col += _canvas._text[row].chunks[i].text.size();
					}
				}
				break;
			} else {
				pos -= _canvas.getLineCharWidth(row); // (row ? 1 : 0);
			}

			row++;
			if ((uint)row >= _canvas._text.size()) {
				row = _canvas._text.size() - 1;
				colX = _canvas._surface->w;
				col = _canvas.getLineCharWidth(row);

				break;
			}
		}
	} else if (pos == 0) {
		colX = col = row = 0;
	} else {
		row = _canvas._text.size() - 1;
		col = _canvas.getLineCharWidth(row);
		// if we don't have any text, then we won't select the whole area.
		if (_canvas._textMaxWidth == 0)
			colX = 0;
		else
			colX = _canvas._textMaxWidth;
	}

	int rowY = _canvas._text[row].y;

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
	for (uint i = 0; i < _canvas._text.size(); i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			res += _canvas._text[i].chunks[j].text;
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
				_cursorCol = _canvas.getLineCharWidth(_cursorRow);
			} else {
				_cursorCol--;
			}
			updateCursorPos();

			return true;

		case Common::KEYCODE_RIGHT:
			if (_cursorCol >= _canvas.getLineCharWidth(_cursorRow)) {
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

			getRowCol(_cursorX, _canvas._text[_cursorRow].y, nullptr, nullptr, &_cursorRow, &_cursorCol);
			updateCursorPos();

			return true;

		case Common::KEYCODE_DOWN:
			if (_cursorRow == getLineCount() - 1)
				return true;

			_cursorRow++;

			getRowCol(_cursorX, _canvas._text[_cursorRow].y, nullptr, nullptr, &_cursorRow, &_cursorCol);
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
			if (_cursorCol >= _canvas.getLineCharWidth(_cursorRow)) {
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
	if (_canvas._text.size() == 0)
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
		index += _canvas.getLineCharWidth(r);
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
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			if (_canvas._text[i].chunks[j].text.empty())
				continue;
			index++;
		}
	}

	int cur = 0;
	for (uint j = 0; j < _canvas._text[row].chunks.size(); j++) {
		if (_canvas._text[row].chunks[j].text.empty())
			continue;
		cur += _canvas._text[row].chunks[j].text.size();
		// Avoid overflowing the word index if we run out of line;
		// it should count as part of the last chunk
		if ((cur <= col) && (j < _canvas._text[row].chunks.size() - 1))
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
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			if (_canvas._text[i].chunks[j].text.empty())
				continue;
			if (_canvas._text[i].chunks[j].getEncodedText().contains(','))
				index++;
		}
	}

	int cur = 0;
	for (uint i = 0; i < _canvas._text[row].chunks.size(); i++) {
		if (_canvas._text[row].chunks[i].text.empty())
			continue;

		for (uint j = 0; j < _canvas._text[row].chunks[i].text.size(); j++) {
			cur++;
			if (cur > col)
				break;
			if (_canvas._text[row].chunks[i].text[j] == ',')
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

Common::U32String MacText::getMouseLink(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;
	y += _scrollPos;

	int row, chunk;
	getRowCol(x, y, nullptr, nullptr, &row, nullptr, &chunk);

	if (chunk < 0)
		return Common::U32String();

	if (!_canvas._text[row].picfname.empty())
		return _canvas._text[row].pictitle;

	if (!_canvas._text[row].chunks[chunk].link.empty())
		return _canvas._text[row].chunks[chunk].link;

	return Common::U32String();
}

int MacTextCanvas::getAlignOffset(int row) {
	int alignOffset = 0;
	if (_textAlignment == kTextAlignRight)
		alignOffset = MAX<int>(0, _maxWidth - getLineWidth(row) - 1);
	else if (_textAlignment == kTextAlignCenter)
		alignOffset = (_maxWidth / 2) - (getLineWidth(row) / 2);
	return alignOffset;
}

void MacText::getRowCol(int x, int y, int *sx, int *sy, int *row, int *col, int *chunk_) {
	int nsx = 0, nsy = 0, nrow = 0, ncol = 0;

	if (y > _canvas._textMaxHeight) {
		x = _canvas._surface->w;
	}

	y = CLIP(y, 0, _canvas._textMaxHeight);

	nrow = _canvas._text.size();
	// use [lb, ub) bsearch here, final answer would be lb
	int lb = 0, ub = nrow;
	while (ub - lb > 1) {
		int mid = (ub + lb) / 2;
		if (_canvas._text[mid].y <= y) {
			lb = mid;
		} else {
			ub = mid;
		}
	}
	nrow = lb;

	nsy = _canvas._text[nrow].y;
	int chunk = -1;

	if (_canvas._text[nrow].chunks.size() > 0) {
		int alignOffset = _canvas.getAlignOffset(nrow) + _canvas._text[nrow].indent + _canvas._text[nrow].firstLineIndent;;

		int width = 0, pwidth = 0;
		int mcol = 0, pmcol = 0;

		for (chunk = 0; chunk < (int)_canvas._text[nrow].chunks.size(); chunk++) {
			pwidth = width;
			pmcol = mcol;
			if (!_canvas._text[nrow].chunks[chunk].text.empty()) {
				width += getStringWidth(_canvas._text[nrow].chunks[chunk], _canvas._text[nrow].chunks[chunk].text);
				mcol += _canvas._text[nrow].chunks[chunk].text.size();
			}

			if (width + alignOffset > x)
				break;
		}

		if (chunk >= (int)_canvas._text[nrow].chunks.size())
			chunk = _canvas._text[nrow].chunks.size() - 1;

		if (chunk_)
			*chunk_ = (int)chunk;

		Common::U32String str = _canvas._text[nrow].chunks[chunk].text;

		ncol = mcol;
		nsx = pwidth;

		for (int i = str.size(); i >= 0; i--) {
			int strw = getStringWidth(_canvas._text[nrow].chunks[chunk], str);
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
	if (chunk_)
		*chunk_ = (int)chunk;
}

// If adjacent chunks have same format, then skip the format definition
// This happens when a long paragraph is split into several lines
#define ADDFORMATTING()                                                                      \
	if (formatted) {                                                                         \
		formatting = Common::U32String(_canvas._text[i].chunks[chunk].toString()); \
		if (formatting != prevformatting) {                                                  \
			res += formatting;                                                               \
			prevformatting = formatting;                                                     \
		}                                                                                    \
	}

Common::U32String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::U32String res("");

	if (endRow == -1)
		endRow = _canvas._text.size() - 1;

	if (endCol == -1)
		endCol = _canvas.getLineCharWidth(endRow);
	if (_canvas._text.empty()) {
		return res;
	}

	startRow = CLIP(startRow, 0, (int)_canvas._text.size() - 1);
	endRow = CLIP(endRow, 0, (int)_canvas._text.size() - 1);

	Common::U32String formatting(""), prevformatting("");

	for (int i = startRow; i <= endRow; i++) {
		// We requested only part of one line
		if (i == startRow && i == endRow) {
			for (uint chunk = 0; chunk < _canvas._text[i].chunks.size(); chunk++) {
				if (_canvas._text[i].chunks[chunk].text.empty()) {
					// skip empty chunks, but keep them formatted,
					// a text input box needs to keep the formatting even when all text is removed.
					ADDFORMATTING();
					continue;
				}

				if (startCol <= 0) {
					ADDFORMATTING();

					if (endCol >= (int)_canvas._text[i].chunks[chunk].text.size())
						res += _canvas._text[i].chunks[chunk].text;
					else
						res += _canvas._text[i].chunks[chunk].text.substr(0, endCol);
				} else if ((int)_canvas._text[i].chunks[chunk].text.size() > startCol) {
					ADDFORMATTING();
					res += _canvas._text[i].chunks[chunk].text.substr(startCol, endCol - startCol);
				}

				startCol -= _canvas._text[i].chunks[chunk].text.size();
				endCol -= _canvas._text[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		// We are at the top line and it is not completely requested
		} else if (i == startRow && startCol != 0) {
			for (uint chunk = 0; chunk < _canvas._text[i].chunks.size(); chunk++) {
				if (_canvas._text[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				if (startCol <= 0) {
					ADDFORMATTING();
					res += _canvas._text[i].chunks[chunk].text;
				} else if ((int)_canvas._text[i].chunks[chunk].text.size() > startCol) {
					ADDFORMATTING();
					res += _canvas._text[i].chunks[chunk].text.substr(startCol);
				}

				startCol -= _canvas._text[i].chunks[chunk].text.size();
			}
			if (newlines && _canvas._text[i].paragraphEnd)
				res += '\n';
		// We are at the end row, and it could be not completely requested
		} else if (i == endRow) {
			for (uint chunk = 0; chunk < _canvas._text[i].chunks.size(); chunk++) {
				if (_canvas._text[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				ADDFORMATTING();

				if (endCol >= (int)_canvas._text[i].chunks[chunk].text.size())
					res += _canvas._text[i].chunks[chunk].text;
				else
					res += _canvas._text[i].chunks[chunk].text.substr(0, endCol);

				endCol -= _canvas._text[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		// We are in the middle of requested range, pass whole line
		} else {
			for (uint chunk = 0; chunk < _canvas._text[i].chunks.size(); chunk++) {
				if (_canvas._text[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				ADDFORMATTING();
				res += _canvas._text[i].chunks[chunk].text;
			}

			if (newlines && _canvas._text[i].paragraphEnd)
				res += '\n';
		}
	}

	return res;
}

// mostly, we refering reshuffleParagraph to implement this function
void MacText::insertTextFromClipboard() {
	int ppos = 0;
	Common::U32String str = _wm->getTextFromClipboard(Common::U32String(_defaultFormatting.toString()), &ppos);

	if (_canvas._text.empty()) {
		_canvas.splitString(str, -1, _defaultFormatting);
	} else {
		int start = _cursorRow, end = _cursorRow;

		while (start && !_canvas._text[start - 1].paragraphEnd)
			start--;

		while (end < (int)_canvas._text.size() - 1 && !_canvas._text[end].paragraphEnd)
			end++;

		for (int i = start; i < _cursorRow; i++)
			ppos += _canvas.getLineCharWidth(i);
		ppos += _cursorCol;

		Common::U32String pre_str = getTextChunk(start, 0, _cursorRow, _cursorCol, true, true);
		Common::U32String sub_str = getTextChunk(_cursorRow, _cursorCol, end, _canvas.getLineCharWidth(end, true), true, true);

		// Remove it from the text
		for (int i = start; i <= end; i++) {
			_canvas._text.remove_at(start);
		}
		_canvas.splitString(pre_str + str + sub_str, start, _defaultFormatting);

		_cursorRow = start;
	}

	warning("FIXME, bad design");
	while (ppos > _canvas.getLineCharWidth(_cursorRow, true)) {
		ppos -= _canvas.getLineCharWidth(_cursorRow, true);
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
	_canvas._text.clear();
	_canvas.splitString(_str, -1, _defaultFormatting);

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
	if (_canvas._text.empty()) {
		appendTextDefault(Common::String(c));
		(*col)++;

		return;
	}

	MacTextLine *line = &_canvas._text[*row];
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

	if (_canvas.getLineWidth(*row) - oldw + chunkw > _canvas._maxWidth) { // Needs reshuffle
		reshuffleParagraph(row, col);
		_fullRefresh = true;
		recalcDims();
		render();
	} else {
		recalcDims();
		_canvas.render(*row, *row);
	}
	for (int i = 0; i < (int)_canvas._text.size(); i++) {
		D(9, "**insertChar line %d isEnd %d", i, _canvas._text[i].paragraphEnd);
		for (int j = 0; j < (int)_canvas._text[i].chunks.size(); j++) {
			D(9, "[%d] \"%s\"",_canvas._text[i].chunks[j].text.size(), Common::toPrintable(_canvas._text[i].chunks[j].text.encode()).c_str());
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
		*col = _canvas.getLineCharWidth(*row - 1);
		(*row)--;

		// formatting matches, glue texts as normal
		if (_canvas._text[*row].lastChunk().equals(_canvas._text[*row + 1].firstChunk())) {
			_canvas._text[*row].lastChunk().text += _canvas._text[*row + 1].firstChunk().text;
			_canvas._text[*row + 1].firstChunk().text.clear();
		} else {
			// formatting doesn't match, move whole chunk
			_canvas._text[*row].chunks.push_back(MacFontRun(_canvas._text[*row + 1].firstChunk()));
			_canvas._text[*row].firstChunk().text.clear();
		}
		_canvas._text[*row].paragraphEnd = false;

		for (uint i = 1; i < _canvas._text[*row + 1].chunks.size(); i++)
			_canvas._text[*row].chunks.push_back(MacFontRun(_canvas._text[*row + 1].chunks[i]));

		_canvas._text.remove_at(*row + 1);
	} else {
		int pos = *col - 1;
		uint ch = _canvas._text[*row].getChunkNum(&pos);

		if (pos == (int)_canvas._text[*row].chunks[ch].text.size())
			pos--;

		_canvas._text[*row].chunks[ch].text.deleteChar(pos);

		(*col)--;
	}

	_canvas._text[*row].width = -1; // flush the cache
}

void MacText::deletePreviousChar(int *row, int *col) {
	if (*col == 0 && *row == 0) // nothing to do
		return;
	deletePreviousCharInternal(row, col);

	for (int i = 0; i < (int)_canvas._text.size(); i++) {
		D(9, "**deleteChar line %d", i);
		for (int j = 0; j < (int)_canvas._text[i].chunks.size(); j++) {
			D(9, "[%d] \"%s\"",_canvas._text[i].chunks[j].text.size(), Common::toPrintable(_canvas._text[i].chunks[j].text.encode()).c_str());
		}
	}
	D(9, "**deleteChar cursor row %d col %d", _cursorRow, _cursorCol);

	reshuffleParagraph(row, col);

	_fullRefresh = true;
	recalcDims();
	render();
}

void MacText::addNewLine(int *row, int *col) {
	if (_canvas._text.empty()) {
		appendTextDefault(Common::String("\n"));
		(*row)++;

		return;
	}

	MacTextLine *line = &_canvas._text[*row];
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

	_canvas._text[*row].width = -1; // flush the cache

	_canvas._text.insert_at(*row + 1, newline);

	(*row)++;
	*col = 0;

	reshuffleParagraph(row, col);

	for (int i = 0; i < (int)_canvas._text.size(); i++) {
		D(9, "** addNewLine line %d", i);
		for (int j = 0; j < (int)_canvas._text[i].chunks.size(); j++) {
			D(9, "[%d] \"%s\"",_canvas._text[i].chunks[j].text.size(), Common::toPrintable(_canvas._text[i].chunks[j].text.encode()).c_str());
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

	while (start && !_canvas._text[start - 1].paragraphEnd)
		start--;

	while (end < (int)_canvas._text.size() - 1 && !_canvas._text[end].paragraphEnd) // stop at last line
		end++;

	// Get character pos within paragraph
	int ppos = 0;

	for (int i = start; i < *row; i++)
		ppos += _canvas.getLineCharWidth(i);

	ppos += *col;

	// Get whole paragraph
	Common::U32String paragraph = getTextChunk(start, 0, end, _canvas.getLineCharWidth(end, true), true, true);

	// Remove it from the text
	for (int i = start; i <= end; i++) {
		_canvas._text.remove_at(start);
	}

	// And now read it
	D(9, "start %d end %d", start, end);
	_canvas.splitString(paragraph, start, _defaultFormatting);

	// Find new pos within paragraph after reshuffling
	*row = start;

	warning("FIXME, bad design");
	while (ppos > _canvas.getLineCharWidth(*row, true)) {
		ppos -= _canvas.getLineCharWidth(*row, true);
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
	if (_canvas._text.empty()) {
		_cursorX = _cursorY = 0;
	} else {
		undrawCursor();

		_cursorRow = MIN<int>(_cursorRow, _canvas._text.size() - 1);

		int alignOffset = _canvas.getAlignOffset(_cursorRow);

		_cursorY = _canvas._text[_cursorRow].y - _scrollPos;
		_cursorX = _canvas.getLineWidth(_cursorRow, false, _cursorCol) + alignOffset;
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

void MacText::setImageArchive(Common::String fname) {
	_imageArchive = Common::makeZipArchive(fname);

	if (!_imageArchive)
		warning("MacText::setImageArchive(): Could not find %s. Images will not be rendered", fname.c_str());
}

const Surface *MacText::getImageSurface(Common::String &fname) {
#ifndef USE_PNG
	warning("MacText::getImageSurface(): PNG support not compiled. Cannot load file %s", fname.c_str());

	return nullptr;
#else
	if (_imageCache.contains(fname))
		return _imageCache[fname]->getSurface();

	if (!_imageArchive) {
		warning("MacText::getImageSurface(): Image Archive was not loaded. Use setImageArchive()");
		return nullptr;
	}

	Common::SeekableReadStream *stream = _imageArchive->createReadStreamForMember(fname);

	if (!stream) {
		warning("MacText::getImageSurface(): Cannot open file %s", fname.c_str());
		return nullptr;
	}

	_imageCache[fname] = new Image::PNGDecoder();

	if (!_imageCache[fname]->loadStream(*stream)) {
		delete _imageCache[fname];

		warning("MacText::getImageSurface(): Cannot load file %s", fname.c_str());

		return nullptr;
	}

	return _imageCache[fname]->getSurface();
#endif // USE_PNG
}

void MacTextCanvas::processTable(int line, int maxWidth) {
	Common::Array<MacTextTableRow> *table = _text[line].table;
	uint numCols = table->front().cells.size();
	uint numRows = table->size();
	Common::Array<int> maxW(numCols), maxL(numCols), colW(numCols), rowH(numRows);
	Common::Array<bool> flex(numCols), wrap(numCols);

	int width = maxWidth * 0.9;
	int gutter = 10;

	// Compute column widths, both minimal and maximal
	for (auto &row : *table) {
		int i = 0;
		for (auto &cell : row.cells) {
			int cW = 0, cL = 0;
			for (uint l = 0; l < cell._text.size(); l++) {
				(void)cell.getLineWidth(l); // calculate it

				cW = MAX(cW, cell._text[l].width);
				cL = MAX(cL, cell._text[l].minWidth);
			}

			maxW[i] = MAX(maxW[i], cW);
			maxL[i] = MAX(maxL[i], cL);

			i++;
		}
	}

	for (uint i = 0; i < numCols; i++) {
		warning("%d: %d - %d", i, maxL[i], maxW[i]);

		wrap[i] = (maxW[i] != maxL[i]);
	}

	int left = width - (numCols - 1) * gutter;
	int avg = left / numCols;
	int nflex = 0;

	// determine whether columns should be flexible and assign
	// width of non-flexible cells
	for (uint i = 0; i < numCols; i++) {
		flex[i] = (maxW[i] > 2 * avg);
		if (flex[i]) {
			nflex++;
		} else {
			colW[i] = maxW[i];
			left -= colW[i];
		}
	}

	// if there is not enough space, make columns that could
	// be word-wrapped flexible, too
	if (left < nflex * avg) {
		for (uint i = 0; i < numCols; i++) {
			if (!flex[i] && wrap[i]) {
				left += colW[i];
				colW[i] = 0;
				flex[i] = true;
				nflex += 1;
			}
		}
	}

	// Calculate weights for flexible columns. The max width
	// is capped at the page width to treat columns that have to
	// be wrapped more or less equal
	int tot = 0;
	for (uint i = 0; i < numCols; i++) {
		if (flex[i]) {
			maxW[i] = MIN(maxW[i], width);
			tot += maxW[i];
		}
	}

	// Now assign the actual width for flexible columns. Make
	// sure that it is at least as long as the longest word length
	for (uint i = 0; i < numCols; i++) {
		if (flex[i]) {
			colW[i] = left * maxW[i] / tot;
			colW[i] = MAX(colW[i], maxL[i]);
			left -= colW[i];
		}
	}

	for (uint i = 0; i < numCols; i++) {
		warning("%d: %d", i, colW[i]);
	}

	int r = 0;
	for (auto &row : *table) {
		int c = 0;
		rowH[r] = 0;
		for (auto &cell : row.cells) {
			cell._maxWidth = colW[c];

			cell.recalcDims();
			cell.reallocSurface();
			cell._surface->clear(_bgcolor);
			cell.render(0, cell._text.size());

			rowH[r] = MAX(rowH[r], cell._textMaxHeight);

			c++;
		}

		r++;
	}

	int tW = 1, tH = 1;
	for (uint i = 0; i < table->size(); i++)
		tH += rowH[i] + gutter * 2 + 1;

	for (uint i = 0; i < table->front().cells.size(); i++)
		tW += colW[i] + gutter * 2 + 1;

	ManagedSurface *surf = new ManagedSurface(tW, tH, _wm->_pixelformat);
	_text[line].tableSurface = surf;
	_text[line].height = tH;
	_text[line].width = tW;
	surf->clear(_bgcolor);

	surf->hLine(0, 0, tW, _fgcolor);
	surf->vLine(0, 0, tH, _fgcolor);

	int y = 1;
	for (uint i = 0; i < table->size(); i++) {
		y += gutter * 2 + rowH[i];
		surf->hLine(0, y, tW, _fgcolor);
		y++;
	}

	int x = 1;
	for (uint i = 0; i < table->front().cells.size(); i++) {
		x += gutter * 2 + colW[i];
		surf->vLine(x, 0, tH, _fgcolor);
		x++;
	}

	r = 0;
	y = 1 + gutter;
	for (auto &row : *table) {
		int c = 0;
		x = 1 + gutter;
		for (auto &cell : row.cells) {
			surf->blitFrom(*cell._surface, Common::Point(x, y));
			x += gutter * 2 + 1 + colW[c];
			c++;
		}
		y += gutter * 2 + 1 + rowH[r];
		r++;
	}
}

} // End of namespace Graphics
