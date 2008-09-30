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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"
#include "kyra/text.h"

#include "common/system.h"
#include "common/endian.h"

namespace Kyra {

TextDisplayer::TextDisplayer(KyraEngine_v1 *vm, Screen *screen) {
	_screen = screen;
	_vm = vm;

	_talkCoords.y = 0x88;
	_talkCoords.x = 0;
	_talkCoords.w = 0;
	_talkMessageY = 0xC;
	_talkMessageH = 0;
	_talkMessagePrinted = false;
}

void TextDisplayer::setTalkCoords(uint16 y) {
	debugC(9, kDebugLevelMain, "TextDisplayer::setTalkCoords(%d)", y);
	_talkCoords.y = y;
}

int TextDisplayer::getCenterStringX(const char *str, int x1, int x2) {
	debugC(9, kDebugLevelMain, "TextDisplayer::getCenterStringX('%s', %d, %d)", str, x1, x2);
	_screen->_charWidth = -2;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	int strWidth = _screen->getTextWidth(str);
	_screen->setFont(curFont);
	_screen->_charWidth = 0;
	int w = x2 - x1 + 1;
	return x1 + (w - strWidth) / 2;
}

int TextDisplayer::getCharLength(const char *str, int len) {
	debugC(9, kDebugLevelMain, "TextDisplayer::getCharLength('%s', %d)", str, len);
	int charsCount = 0;
	if (*str) {
		_screen->_charWidth = -2;
		Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
		int i = 0;
		while (i <= len && *str) {
			uint c = *str++;
			c &= 0xFF;
			if (c >= 0x7F && _vm->gameFlags().lang == Common::JA_JPN) {
				c = READ_LE_UINT16(str - 1);
				++str;
			}
			i += _screen->getCharWidth(c);
			++charsCount;
		}
		_screen->setFont(curFont);
		_screen->_charWidth = 0;
	}
	return charsCount;
}

int TextDisplayer::dropCRIntoString(char *str, int offs) {
	debugC(9, kDebugLevelMain, "TextDisplayer::dropCRIntoString('%s', %d)", str, offs);
	int pos = 0;
	str += offs;
	while (*str) {
		if (*str == ' ') {
			*str = '\r';
			return pos;
		}
		++str;
		++pos;
	}
	return 0;
}

char *TextDisplayer::preprocessString(const char *str) {
	debugC(9, kDebugLevelMain, "TextDisplayer::preprocessString('%s')", str);
	if (str != _talkBuffer) {
		assert(strlen(str) < sizeof(_talkBuffer) - 1);
		strcpy(_talkBuffer, str);
	}
	char *p = _talkBuffer;
	while (*p) {
		if (*p == '\r') {
			return _talkBuffer;
		}
		++p;
	}
	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	int textWidth = _screen->getTextWidth(p);
	_screen->_charWidth = 0;
	if (textWidth > 176) {
		if (textWidth > 352) {
			int count = getCharLength(p, textWidth / 3);
			int offs = dropCRIntoString(p, count);
			p += count + offs;
			_screen->_charWidth = -2;
			textWidth = _screen->getTextWidth(p);
			_screen->_charWidth = 0;
			count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		} else {
			int count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		}
	}
	_screen->setFont(curFont);
	return _talkBuffer;
}

int TextDisplayer::buildMessageSubstrings(const char *str) {
	debugC(9, kDebugLevelMain, "TextDisplayer::buildMessageSubstrings('%s')", str);
	int currentLine = 0;
	int pos = 0;
	while (*str) {
		if (*str == '\r') {
			assert(currentLine < TALK_SUBSTRING_NUM);
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
			++currentLine;
			pos = 0;
		} else {
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = *str;
			++pos;
			if (pos >= TALK_SUBSTRING_LEN - 2)
				pos = TALK_SUBSTRING_LEN - 2;
		}
		++str;
	}
	_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
	return currentLine + 1;
}

int TextDisplayer::getWidestLineWidth(int linesCount) {
	debugC(9, kDebugLevelMain, "TextDisplayer::getWidestLineWidth(%d)", linesCount);
	int maxWidth = 0;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	for (int l = 0; l < linesCount; ++l) {
		int w = _screen->getTextWidth(&_talkSubstrings[l * TALK_SUBSTRING_LEN]);
		if (maxWidth < w) {
			maxWidth = w;
		}
	}
	_screen->setFont(curFont);
	_screen->_charWidth = 0;
	return maxWidth;
}

void TextDisplayer::calcWidestLineBounds(int &x1, int &x2, int w, int cx) {
	debugC(9, kDebugLevelMain, "TextDisplayer::calcWidestLineBounds(%d, %d)", w, cx);
	x1 = cx - w / 2;
	if (x1 + w >= Screen::SCREEN_W - 12) {
		x1 = Screen::SCREEN_W - 12 - w - 1;
	} else if (x1 < 12) {
		x1 = 12;
	}
	x2 = x1 + w + 1;
}

void TextDisplayer::restoreTalkTextMessageBkgd(int srcPage, int dstPage) {
	debugC(9, kDebugLevelMain, "TextDisplayer::restoreTalkTextMessageBkgd(%d, %d)", srcPage, dstPage);
	if (_talkMessagePrinted) {
		_talkMessagePrinted = false;
		_screen->copyRegion(_talkCoords.x, _talkCoords.y, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, srcPage, dstPage);
	}
}

void TextDisplayer::printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printTalkTextMessage('%s', %d, %d, %d, %d, %d)", text, x, y, color, srcPage, dstPage);
	char *str = preprocessString(text);
	int lineCount = buildMessageSubstrings(str);
	int top = y - lineCount * 10;
	if (top < 0) {
		top = 0;
	}
	_talkMessageY = top;
	_talkMessageH = lineCount * 10;
	int w = getWidestLineWidth(lineCount);
	int x1, x2;
	calcWidestLineBounds(x1, x2, w, x);
	_talkCoords.x = x1;
	_talkCoords.w = w + 2;
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkCoords.y, _talkCoords.w, _talkMessageH, srcPage, dstPage, Screen::CR_NO_P_CHECK);
	int curPage = _screen->_curPage;
	_screen->_curPage = srcPage;
	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		char *msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		int left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, 0);
	}
	_screen->_curPage = curPage;
	_talkMessagePrinted = true;
}

void TextDisplayer::printIntroTextMessage(const char *text, int x, int y, uint8 col1, uint8 col2, uint8 col3, int dstPage, Screen::FontId font) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printIntroTextMessage('%s', %d, %d, %d, %d, %d, %d, %d)",
			text, x, y, col1, col2, col3, dstPage, font);
	char *str = preprocessString(text);
	int lineCount = buildMessageSubstrings(str);
	int top = y - lineCount * 10;
	if (top < 0) {
		top = 0;
	}
	_talkMessageY = top;
	_talkMessageH = lineCount * 10;
	int w = getWidestLineWidth(lineCount);
	int x1, x2;
	calcWidestLineBounds(x1, x2, w, x);
	_talkCoords.x = x1;
	_talkCoords.w = w + 2;
	int curPage = _screen->setCurPage(dstPage);

	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		char *msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		int left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, col1, col2, col3, font);
	}
	_screen->_curPage = curPage;
	_talkMessagePrinted = true;
}

void TextDisplayer::printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2, Screen::FontId font) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printText('%s', %d, %d, %d, %d, %d)", str, x, y, c0, c1, c2);
	uint8 colorMap[] = { 0, 15, 12, 12 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	Screen::FontId curFont = _screen->setFont(font);
	_screen->_charWidth = -2;
	_screen->printText(str, x, y, c0, c2);
	_screen->_charWidth = 0;
	_screen->setFont(curFont);
}

void TextDisplayer::printCharacterText(const char *text, int8 charNum, int charX) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printCharacterText('%s', %d, %d)", text, charNum, charX);
	uint8 colorTable[] = {0x0F, 0x9, 0x0C9, 0x80, 0x5, 0x81, 0x0E, 0xD8, 0x55, 0x3A, 0x3a};
	int top, left, x1, x2, w, x;
	char *msg;

	uint8 color = colorTable[charNum];
	text = preprocessString(text);
	int lineCount = buildMessageSubstrings(text);
	w = getWidestLineWidth(lineCount);
	x = charX;
	calcWidestLineBounds(x1, x2, w, x);

	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, 0);
	}
}
} // end of namespace Kyra
