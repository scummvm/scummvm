/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software(0), you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation(0), either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY(0), without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program(0), if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/gfx/text_control.h"
#include "titanic/support/strings.h"
#include "titanic/titanic.h"

namespace Titanic {

CTextControl::CTextControl(uint count) :
		_stringsMerged(false), _maxCharsPerLine(-1), _lineCount(0),
		_displayEndCharIndex(-1), _unused1(0), _unused2(0), _unused3(0),
		_backR(0xff), _backG(0xff), _backB(0xff),
		_textR(0), _textG(0), _textB(200),
		_fontNumber(0), _npcFlag(0), _npcId(0), _hasBorder(true),
		_scrollTop(0), _textCursor(nullptr) {
	setupArrays(count);
}

void CTextControl::setupArrays(int count) {
	freeArrays();
	if (count < 10 || count > 60)
		count = 10;
	_array.resize(count);
}

void CTextControl::freeArrays() {
	_array.clear();
}

void CTextControl::setup() {
	for (int idx = 0; idx < (int)_array.size(); ++idx) {
		_array[idx]._line.clear();
		setLineColor(idx, _textR, _textG, _textB);
		_array[idx]._string3.clear();
	}

	_lineCount = 0;
	_stringsMerged = false;
}

void CTextControl::setLineColor(uint lineNum, uint col) {
	setLineColor(lineNum, col & 0xff, (col >> 8) & 0xff, (col >> 16) & 0xff);
}

void CTextControl::setLineColor(uint lineNum, byte r, byte g, byte b) {
	_array[lineNum]._rgb = getColorText(r, g, b);
	_stringsMerged = false;
}

CString CTextControl::getColorText(byte r, byte g, byte b) {
	char buffer[6];
	if (!r)
		r = 1;
	if (!g)
		g = 1;
	if (!b)
		b = 1;

	buffer[0] = TEXTCMD_SET_COLOR;
	buffer[1] = r;
	buffer[2] = g;
	buffer[3] = b;
	buffer[4] = TEXTCMD_SET_COLOR;
	buffer[5] = '\0';

	return CString(buffer);
}

void CTextControl::load(SimpleFile *file, int param) {
	if (!param) {
		uint numLines = file->readNumber();
		int charsPerLine = file->readNumber();
		uint count = file->readNumber();
		_bounds = file->readRect();
		_unused1 = file->readNumber();
		_unused2 = file->readNumber();
		_unused3 = file->readNumber();
		_backR = file->readNumber();
		_backG = file->readNumber();
		_backB = file->readNumber();
		_textR = file->readNumber();
		_textG = file->readNumber();
		_textB = file->readNumber();
		_hasBorder = file->readNumber() != 0;
		_scrollTop = file->readNumber();

		setMaxCharsPerLine(charsPerLine);
		resize(numLines);
		_lineCount = (count == 0) ? 0 : count - 1;

		assert(_array.size() >= count);
		for (uint idx = 0; idx < count; ++idx) {
			_array[idx]._line = file->readString();
			_array[idx]._rgb = file->readString();
			_array[idx]._string3 = file->readString();
		}
	}
}

void CTextControl::save(SimpleFile *file, int indent) {
	int numLines = _lineCount + 1;

	file->writeNumberLine(_array.size(), indent);
	file->writeNumberLine(_maxCharsPerLine, indent);
	file->writeNumberLine(numLines, indent);

	file->writeRect(_bounds, indent);
	file->writeNumberLine(_unused1, indent);
	file->writeNumberLine(_unused2, indent);
	file->writeNumberLine(_unused3, indent);
	file->writeNumberLine(_backR, indent);
	file->writeNumberLine(_backG, indent);
	file->writeNumberLine(_backB, indent);
	file->writeNumberLine(_textR, indent);
	file->writeNumberLine(_textG, indent);
	file->writeNumberLine(_textB, indent);
	file->writeNumberLine(_hasBorder, indent);
	file->writeNumberLine(_scrollTop, indent);

	for (int idx = 0; idx < numLines; ++idx) {
		file->writeQuotedLine(_array[idx]._line, indent);
		file->writeQuotedLine(_array[idx]._rgb, indent);
		file->writeQuotedLine(_array[idx]._string3, indent);
	}
}

void CTextControl::draw(CScreenManager *screenManager) {
	Rect tempRect = _bounds;

	if (_hasBorder) {
		// Create border effect
		// Top edge
		tempRect.bottom = tempRect.top + 1;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);

		// Bottom edge
		tempRect.top = _bounds.bottom - 1;
		tempRect.bottom = _bounds.bottom;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);

		// Left edge
		tempRect = _bounds;
		tempRect.right = tempRect.left + 1;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);

		// Right edge
		tempRect = _bounds;
		tempRect.left = tempRect.right - 1;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);
	}

	getTextHeight(screenManager);

	tempRect = _bounds;
	tempRect.grow(-2);
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);

	_displayEndCharIndex = screenManager->writeString(SURFACE_BACKBUFFER, tempRect, _scrollTop, _lines, _textCursor);

	screenManager->setFontNumber(oldFontNumber);
}

void CTextControl::mergeStrings() {
	if (!_stringsMerged) {
		_lines.clear();

		for (int idx = 0; idx <= _lineCount; ++idx) {
			CString line = _array[idx]._rgb + _array[idx]._string3 +
				_array[idx]._line + "\n";
			_lines += line;
		}

		_stringsMerged = true;
	}
}

void CTextControl::resize(uint count) {
	if (!count || _array.size() == count)
		return;
	_array.clear();
	_array.resize(count);
}

CString CTextControl::getText() const {
	CString result = "";
	for (int idx = 0; idx <= _lineCount; ++idx)
		result += _array[idx]._line;

	return result;
}

void CTextControl::setText(const CString &str) {
	setup();
	appendText(str);
}

void CTextControl::setText(StringId stringId) {
	setText(g_vm->_strings[stringId]);
}

void CTextControl::appendText(const CString &str) {
	int lineSize = _array[_lineCount]._line.size();
	int strSize = str.size();

	if (_maxCharsPerLine == -1) {
		// No limit on horizontal characters, so append string to current line
		_array[_lineCount]._line += str;
	} else if ((lineSize + strSize) <= _maxCharsPerLine) {
		// New string fits into line, so add it on
		_array[_lineCount]._line += str;
	} else {
		// Only add part of the str up to the maximum allowed limit for line
		_array[_lineCount]._line += str.left(_maxCharsPerLine - lineSize);
	}

	updateStr3(_lineCount);
	_stringsMerged = false;
}

void CTextControl::setColor(uint col) {
	_textR = col & 0xff;
	_textG = (col >> 8) & 0xff;
	_textB = (col >> 16) & 0xff;
}

void CTextControl::setColor(byte r, byte g, byte b) {
	_textR = r;
	_textG = g;
	_textB = b;
}

void CTextControl::remapColors(uint count, uint *srcColors, uint *destColors) {
	for (int lineNum = 0; lineNum <= _lineCount; ++lineNum) {
		if (_array[lineNum]._rgb.empty())
			continue;

		// Get the rgb values
		uint r = _array[lineNum]._rgb[1];
		uint g = _array[lineNum]._rgb[2];
		uint b = _array[lineNum]._rgb[3];
		uint color = r | (g << 8) | (b << 16);

		for (uint index = 0; index < count; ++index) {
			if (color == srcColors[index]) {
				// Found a match, so replace the color
				setLineColor(lineNum, destColors[index]);
				break;
			}
		}
	}

	_stringsMerged = false;
}

void CTextControl::setMaxCharsPerLine(int maxChars) {
	if (maxChars >= -1 && maxChars < 257)
		_maxCharsPerLine = maxChars;
}

void CTextControl::updateStr3(int lineNum) {
	if (_npcFlag > 0 && _npcId > 0) {
		char line[5];
		line[0] = line[3] = TEXTCMD_NPC;
		line[1] = _npcFlag;
		line[2] = _npcId;
		line[4] = '\0';
		_array[lineNum]._string3 = CString(line);

		_stringsMerged = false;
		_npcFlag = _npcId = 0;
	}
}

int CTextControl::getTextWidth(CScreenManager *screenManager) {
	mergeStrings();
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	int textWidth = screenManager->stringWidth(_lines);
	screenManager->setFontNumber(oldFontNumber);

	return textWidth;
}

int CTextControl::getTextHeight(CScreenManager *screenManager) {
	mergeStrings();
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	int textHeight = screenManager->getTextBounds(_lines, _bounds.width() - 4);
	screenManager->setFontNumber(oldFontNumber);

	return textHeight;
}

void CTextControl::deleteLastChar() {
	if (!_array[_lineCount]._line.empty()) {
		_array[_lineCount]._line.deleteLastChar();
		_stringsMerged = false;
	}
}

void CTextControl::setNPC(int npcFlag, int npcId) {
	_npcFlag = npcFlag;
	_npcId = npcId;
}

void CTextControl::scrollUp(CScreenManager *screenManager) {
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	_scrollTop -= screenManager->getFontHeight();
	constrainScrollUp(screenManager);
	screenManager->setFontNumber(oldFontNumber);
}

void CTextControl::scrollDown(CScreenManager *screenManager) {
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	_scrollTop += screenManager->getFontHeight();
	constrainScrollDown(screenManager);
	screenManager->setFontNumber(oldFontNumber);
}

void CTextControl::scrollUpPage(CScreenManager *screenManager) {
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	_scrollTop -= getPageHeight(screenManager);
	constrainScrollUp(screenManager);
	screenManager->setFontNumber(oldFontNumber);
}

void CTextControl::scrollDownPage(CScreenManager *screenManager) {
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	_scrollTop += getPageHeight(screenManager);
	constrainScrollDown(screenManager);
	screenManager->setFontNumber(oldFontNumber);
}

void CTextControl::scrollToTop(CScreenManager *screenManager) {
	_scrollTop = 0;
}

void CTextControl::scrollToBottom(CScreenManager *screenManager) {
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	_scrollTop = getTextHeight(screenManager);
	constrainScrollDown(screenManager);
	screenManager->setFontNumber(oldFontNumber);
}

void CTextControl::constrainScrollUp(CScreenManager *screenManager) {
	if (_scrollTop < 0)
		_scrollTop = 0;
}

void CTextControl::constrainScrollDown(CScreenManager *screenManager) {
	// Figure out the maximum scroll amount allowed
	int maxScroll = getTextHeight(screenManager) - _bounds.height() - 4;
	if (maxScroll < 0)
		maxScroll = 0;

	if (_scrollTop > maxScroll)
		_scrollTop = maxScroll;
}

int CTextControl::getPageHeight(CScreenManager *screenManager) {
	int textHeight = _bounds.height();
	int oldFontNumber = screenManager->setFontNumber(_fontNumber);
	int fontHeight = screenManager->getFontHeight();
	screenManager->setFontNumber(oldFontNumber);

	if (fontHeight) {
		int lines = textHeight / fontHeight;
		if (lines > 1)
			--lines;
		return lines * fontHeight;
	} else {
		return 0;
	}
}

void CTextControl::addLine(const CString &str) {
	addLine(str, _textR, _textG, _textB);
}

void CTextControl::addLine(const CString &str, uint color) {
	addLine(str, color & 0xff, (color >> 8) & 0xff,
		(color >> 16) & 0xff);
}

void CTextControl::addLine(const CString &str, byte r, byte g, byte b) {
	if (_lineCount == ((int)_array.size() - 1)) {
		// Lines array is full
		if (_array.size() > 1) {
			// Delete the oldest line, and add a new entry at the end
			_array.remove_at(0);
			_array.resize(_array.size() + 1);
		}

		--_lineCount;
	}

	setLineColor(_lineCount, r, g, b);
	appendText(str);
	++_lineCount;
}

bool CTextControl::handleKey(char c) {
	switch (c) {
	case (char)Common::KEYCODE_BACKSPACE:
		deleteLastChar();
		break;

	case (char)Common::KEYCODE_RETURN:
		return true;

	default:
		if ((byte)c >= 32 && (byte)c <= 127)
			appendText(CString(c, 1));
		break;
	}

	return false;
}

void CTextControl::showCursor(int mode) {
	CScreenManager *screenManager = CScreenManager::setCurrent();
	_textCursor = screenManager->_textCursor;
	if (_textCursor) {
		_textCursor->setPos(Point(0, 0));
		_textCursor->setSize(Point(2, 10));
		_textCursor->setColor(0, 0, 0);
		_textCursor->setBlinkRate(300);
		_textCursor->setMode(mode);
		_textCursor->setBounds(_bounds);
		_textCursor->show();
	}
}

void CTextControl::hideCursor() {
	if (_textCursor) {
		_textCursor->setMode(-1);
		_textCursor->hide();
		_textCursor = nullptr;
	}
}

int CTextControl::getNPCNum(uint ident, uint startIndex) {
	if (!_stringsMerged) {
		mergeStrings();
		if (!_stringsMerged)
			return -1;
	}

	uint size = _lines.size();
	if (startIndex < 5 || startIndex >= size)
		return -1;

	// Loop backwards from the starting index to find an NPC ident sequence
	for (const char *strP = _lines.c_str() + startIndex;
			strP >= (_lines.c_str() + 5); --strP) {
		if (*strP == 26) {
			byte id = *(strP - 2);
			if (id == ident)
				return *(strP - 1);
			strP -= 3;
		} else if (*strP == 27) {
			strP -= 4;
		}
	}

	return -1;
}

void CTextControl::setFontNumber(int fontNumber) {
	if (fontNumber >= 0 && fontNumber <= 2)
		_fontNumber = fontNumber;
}

} // End of namespace Titanic
