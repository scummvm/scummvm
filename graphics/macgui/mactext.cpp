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

#include "graphics/macgui/mactext.h"

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

	if (i && i == chunks.size()) {
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

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
		byte r, g, b;
		if (_wm->_pixelformat.bytesPerPixel == 4) {
			_wm->decomposeColor<uint32>(fgcolor, r, g, b);
		} else {
			_wm->decomposeColor<byte>(fgcolor, r, g, b);
		}
		_defaultFormatting.setValues(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), r, g, b);
	} else {
		_defaultFormatting.font = NULL;
	}

	init(fgcolor, bgcolor, maxWidth, textAlignment, interlinear, textShadow, true);
}

// NOTE: This constructor and the one afterward are for MacText engines that don't use widgets. This is the classic was MacText was constructed.
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *macFont, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, bool fixedDims) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0),
	_macFont(macFont) {

	_str = s;

	_fixedDims = fixedDims;
	_wm = wm;

	if (macFont) {
		_defaultFormatting = MacFontRun(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);
		_defaultFormatting.font = wm->_fontMan->getFont(*macFont);
		byte r, g, b;
		_wm->_pixelformat.colorToRGB(fgcolor, r, g, b);
		_defaultFormatting.setValues(_wm, macFont->getId(), macFont->getSlant(), macFont->getSize(), r, g, b);
	} else {
		_defaultFormatting.font = NULL;
	}

	init(fgcolor, bgcolor, maxWidth, textAlignment, interlinear, 0, true);
}

// Working with plain Font
MacText::MacText(const Common::U32String &s, MacWindowManager *wm, const Font *font, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, bool fixedDims) :
	MacWidget(nullptr, 0, 0, 0, 0, wm, false, 0, 0, 0),
	_macFont(nullptr) {

	_str = s;

	_fixedDims = fixedDims;
	_wm = wm;

	if (font) {
		_defaultFormatting = MacFontRun(_wm, font, 0, font->getFontHeight(), 0, 0, 0);
		_defaultFormatting.font = font;
	} else {
		_defaultFormatting.font = NULL;
	}

	init(fgcolor, bgcolor, maxWidth, textAlignment, interlinear, 0, false);
}

void MacText::init(uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, uint16 textShadow, bool macFontMode) {
	_fullRefresh = true;

	_canvas._maxWidth = maxWidth;
	_canvas._textAlignment = textAlignment;
	_canvas._textShadow = textShadow;
	_canvas._interLinear = interlinear;
	_canvas._wm = _wm;
	_canvas._tfgcolor = fgcolor;
	_canvas._tbgcolor = bgcolor;
	_canvas._macFontMode = macFontMode;
	_canvas._macText = this;
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
		_canvas._tfgcolor = colorFontRun.fgcolor;
		colorFontRun.text.clear();
		debug(9, "Reading fg color though text, instead of the argument, read %d", _canvas._tfgcolor);
		_defaultFormatting = colorFontRun;
		_defaultFormatting.wm = _wm;
	}

	_currentFormatting = _defaultFormatting;
	_composeSurface->clear(_canvas._tbgcolor);

	_cursorSurface = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface->clear(_canvas._tfgcolor);
	_cursorSurface2 = new ManagedSurface(1, kCursorMaxHeight, _wm->_pixelformat);
	_cursorSurface2->clear(_canvas._tbgcolor);

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

void MacText::setMaxWidth(int maxWidth) {
	if (maxWidth == _canvas._maxWidth)
		return;

	if (maxWidth < 0) {
		warning("trying to set maxWidth to %d", maxWidth);
		return;
	}

	for (uint i = 0; i < _canvas._text.size(); i++) {
		if (_canvas._text[i].table) {
			// TODO
			debug(0, "MacText::setMaxWidth(): Skipping resize for MacText with tables");
			return;
		}
	}

	// keep the cursor pos
	int ppos = 0;
	for (int i = 0; i < _cursorRow; i++)
		ppos += _canvas.getLineCharWidth(i);
	ppos += _cursorCol;

	_canvas.setMaxWidth(maxWidth, _defaultFormatting);

	// restore the cursor pos
	_cursorRow = 0;
	while (ppos > _canvas.getLineCharWidth(_cursorRow, true)) {
		ppos -= _canvas.getLineCharWidth(_cursorRow, true);

		if (_cursorRow >= (int)_canvas._text.size() - 1)
			break;

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
	_canvas._tbgcolor = bg;
	_canvas._tfgcolor = fg;
	// also set the cursor color
	_cursorSurface->clear(_canvas._tfgcolor);
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
			_canvas._text[i].chunks[j].textSlant = textSlant;
		}
	}

	_fullRefresh = true;
	render();
	_contentIsDirty = true;
}

// Return the number of rows of text in the rendered output.
// This means e.g. a line of text that wraps will count as 2 or more rows.
int MacText::getRowCount() {
	return (int)_canvas._text.size();
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

void MacText::render() {
	if (_fullRefresh) {
		_canvas._surface->clear(_canvas._tbgcolor);
		if (_canvas._textShadow)
			_canvas._shadowSurface->clear(_canvas._tbgcolor);

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

int MacText::getLastLineWidth() {
	if (_canvas._text.size() == 0)
		return 0;

	return _canvas.getLineWidth(_canvas._text.size() - 1, true);
}

int MacText::getLineHeight(int line) {
	return _canvas.getLineHeight(line);
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
	clearChunkInput();

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

void MacText::clearChunkInput() {
	int canvasTextSize = _canvas._text.size() - 1;

	if (canvasTextSize >= 0 && _editable) {
		int lastChunkIdx = _canvas._text[canvasTextSize].chunks.size() - 1;

		if (lastChunkIdx >= 0)
			_canvas._text[canvasTextSize].chunks[lastChunkIdx].text = "";
	}
}

void MacText::clearText() {
	_contentIsDirty = true;
	_canvas._text.clear();
	_str.clear();

	if (_canvas._surface)
		_canvas._surface->clear(_canvas._tbgcolor);

	recalcDims();

	_cursorRow = _cursorCol = 0;
	updateCursorPos();
}

void MacText::removeLastLine() {
	if (!_canvas._text.size())
		return;

	int h = getLineHeight(_canvas._text.size() - 1) + _canvas._interLinear;

	_canvas._surface->fillRect(Common::Rect(0, _canvas._textMaxHeight - h, _canvas._surface->w, _canvas._textMaxHeight), _canvas._tbgcolor);

	_canvas._text.pop_back();
	_canvas._textMaxHeight -= h;
}

void MacText::draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff) {
	if (_canvas._text.empty())
		return;

	render();

	if (x + w < _canvas._surface->w || y + h < _canvas._surface->h)
		g->fillRect(Common::Rect(x + xoff, y + yoff, x + w + xoff, y + h + yoff), _canvas._tbgcolor);

	// blit shadow surface first
	if (_canvas._textShadow)
		g->blitFrom(*_canvas._shadowSurface, Common::Rect(MIN<int>(_canvas._surface->w, x), MIN<int>(_canvas._surface->h, y), MIN<int>(_canvas._surface->w, x + w), MIN<int>(_canvas._surface->h, y + h)), Common::Point(xoff + _canvas._textShadow, yoff + _canvas._textShadow));

	g->transBlitFrom(*_canvas._surface, Common::Rect(MIN<int>(_canvas._surface->w, x), MIN<int>(_canvas._surface->h, y), MIN<int>(_canvas._surface->w, x + w), MIN<int>(_canvas._surface->h, y + h)), Common::Point(xoff, yoff), _canvas._tbgcolor);

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
		_composeSurface->clear(_canvas._tbgcolor);

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
			if (*ptr == _canvas._tfgcolor)
				*ptr = _canvas._tbgcolor;
			else
				*ptr = _canvas._tfgcolor;
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

	return _canvas.getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, formatted, newlines);
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

	int rowY = 0;
	if (!_canvas._text.empty())
		rowY = _canvas._text[row].y;

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
	return _canvas.getTextChunk(_editableRow, 0, -1, -1);
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

	Common::U32String selection = _canvas.getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, true, true);

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

	int index = 1;
	bool inWhitespace = true;
	// getMouseWord:
	// - starts from index 1
	// - a word goes from the first leading whitespace character
	//   up until the end of the word
	// - trailing whitespace or empty space at the end of a line
	//   counts as part of the word on the next line
	// - empty space at the end of the text counts as a word
	for (int i = 0; i < row; i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			if (_canvas._text[i].chunks[j].text.empty())
				continue;
			Common::String data = _canvas._text[i].chunks[j].getEncodedText();
			for (auto it : data) {
				if (it == ' ' && !inWhitespace) {
					index++;
					inWhitespace = true;
				} else if (it != ' ' && inWhitespace) {
					inWhitespace = false;
				}
			}
		}
	}

	int cur = 0;
	for (uint j = 0; j < _canvas._text[row].chunks.size(); j++) {
		if (_canvas._text[row].chunks[j].text.empty())
			continue;
		Common::String data = _canvas._text[row].chunks[j].getEncodedText();
		for (auto it : data) {
			cur++;
			if (it == ' ' && !inWhitespace) {
				index++;
				inWhitespace = true;
			} else if (it != ' ' && inWhitespace) {
				inWhitespace = false;
			}
			if (cur > col)
				break;
		}
		if (cur > col)
			break;
	}

	return index;
}

int MacText::getMouseItem(int x, int y) {
	Common::Point offset = calculateOffset();
	x -= getDimensions().left - offset.x;
	y -= getDimensions().top - offset.y;
	y += _scrollPos;

	int dx, dy, row, col;
	getRowCol(x, y, &dx, &dy, &row, &col);

	// getMouseItem
	// - starts from index 1
	// - an item goes from the first non-comma character up until a comma
	// - trailing whitespace or empty space at the end of a line
	//   counts as part of the item on the next line
	// - empty space at the end of the text counts as an item
	int index = 1;
	bool onComma = false;
	for (int i = 0; i < row; i++) {
		for (uint j = 0; j < _canvas._text[i].chunks.size(); j++) {
			if (_canvas._text[i].chunks[j].text.empty())
				continue;
			Common::String data = _canvas._text[i].chunks[j].getEncodedText();
			for (auto it : data) {
				if (onComma) {
					index += 1;
					onComma = false;
				}
				if (it == ',') {
					onComma = true;
				}
			}
		}
	}

	int cur = 0;
	for (uint i = 0; i < _canvas._text[row].chunks.size(); i++) {
		if (_canvas._text[row].chunks[i].text.empty())
			continue;
		Common::String data = _canvas._text[row].chunks[i].getEncodedText();
		for (auto it : data) {
			if (onComma) {
				index += 1;
				onComma = false;
			}
			if (it == ',') {
				onComma = true;
			}
			cur++;
			if (cur > col)
				break;
		}
		if (cur > col)
			break;
	}

	return index;
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
		int alignOffset = _canvas.getAlignOffset(nrow) + _canvas._text[nrow].indent + _canvas._text[nrow].firstLineIndent;

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

Common::U32String MacText::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	return _canvas.getTextChunk(startRow, startCol, endRow, endCol, formatted, newlines);
}

// mostly, we referring reshuffleParagraph to implement this function
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

		Common::U32String pre_str = _canvas.getTextChunk(start, 0, _cursorRow, _cursorCol, true, true);
		Common::U32String sub_str = _canvas.getTextChunk(_cursorRow, _cursorCol, end, _canvas.getLineCharWidth(end, true), true, true);

		// Remove it from the text
		for (int i = start; i <= end; i++) {
			_canvas._text.remove_at(start);
		}
		_canvas.splitString(pre_str + str + sub_str, start, _defaultFormatting);

		_cursorRow = start;
	}

	while (ppos > _canvas.getLineCharWidth(_cursorRow, true)) {
		ppos -= _canvas.getLineCharWidth(_cursorRow, true);

		if (_cursorRow == (int)_canvas._text.size() - 1)
			break;

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
	if (_canvas._text.empty() || _canvas._text[*row].chunks.empty()) {
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
		_canvas.reshuffleParagraph(row, col, _defaultFormatting);
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

	_canvas.reshuffleParagraph(&row, &col, _defaultFormatting);

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

#if DEBUG
		D(9, "MacText::deletePreviousCharInternal: Chunks: ");
		for (auto &ch : _canvas._text[*row].chunks)
			ch.debugPrint();

		D(9, "");
#endif
		// formatting matches, glue texts as normal
		if (_canvas._text[*row].lastChunk().equals(_canvas._text[*row + 1].firstChunk())) {
			_canvas._text[*row].lastChunk().text += _canvas._text[*row + 1].firstChunk().text;
			_canvas._text[*row + 1].firstChunk().text.clear();
		} else {
			// formatting doesn't match, move whole chunk
			_canvas._text[*row].chunks.push_back(MacFontRun(_canvas._text[*row + 1].firstChunk()));
			_canvas._text[*row].firstChunk().text.clear();
		}
		_canvas._text[*row].paragraphEnd = _canvas._text[*row + 1].paragraphEnd;

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

	_canvas.reshuffleParagraph(row, col, _defaultFormatting);

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

#if DEBUG
	D(9, "MacText::addNewLine: Chunks: ");
	for (auto &c : line->chunks)
		c.debugPrint();

	D(9, "");
#endif

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

	_canvas.reshuffleParagraph(row, col, _defaultFormatting);

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

} // End of namespace Graphics
