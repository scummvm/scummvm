/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
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

#include "kyra/text/text.h"
#include "kyra/kyra_v1.h"

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
	_langExtraSpacing = (vm->gameFlags().lang == Common::KO_KOR) ? 2 : 0;
	_lineBreakChar = (_vm->gameFlags().platform == Common::kPlatformMacintosh) ? '\n' : '\r';
	memset(_talkSubstrings, 0, sizeof(_talkSubstrings));
	memset(_talkBuffer, 0, sizeof(_talkBuffer));
}

void TextDisplayer::setTalkCoords(uint16 y) {
	_talkCoords.y = y;
}

int TextDisplayer::getCenterStringX(const Common::String &str, int x1, int x2) {
	_screen->_charSpacing = -2;
	int strWidth = _screen->getTextWidth(str.c_str());
	_screen->_charSpacing = 0;
	int w = x2 - x1 + 1;
	return x1 + (w - strWidth) / 2;
}

int TextDisplayer::getCharLength(const char *str, int len) {
	int charsCount = 0;
	if (*str) {
		_screen->_charSpacing = -2;
		int i = 0;
		while (i <= len && *str) {
			uint c = *str++;
			c &= 0xFF;
			if (c > 0x7F && (_vm->gameFlags().lang == Common::JA_JPN || _vm->gameFlags().lang == Common::KO_KOR)) {
				c = READ_LE_UINT16(str - 1);
				++charsCount;
				++str;
			}
			i += _screen->getCharWidth(c);
			++charsCount;
		}
		_screen->_charSpacing = 0;
	}
	return charsCount;
}

int TextDisplayer::dropCRIntoString(char *str, int offs) {
	int pos = 0;
	str += offs;
	while (*str) {
		if (*str == ' ') {
			*str = _lineBreakChar;
			return pos;
		}
		++str;
		++pos;
	}
	return 0;
}

char *TextDisplayer::preprocessString(const char *str) {
	if (str != _talkBuffer) {
		assert(strlen(str) < sizeof(_talkBuffer) - 1);
		Common::strlcpy(_talkBuffer, str, sizeof(_talkBuffer));
	}

	if (_vm->gameFlags().lang == Common::ZH_TWN)
		return _talkBuffer;

	char *p = _talkBuffer;
	while (*p) {
		if (*p == _lineBreakChar) {
			return _talkBuffer;
		}
		++p;
	}
	p = _talkBuffer;

	static const uint16 limDef[2] = { 176, 352 };
	static const uint16 limKor[2] = { 240, 480 };
	const uint16 *lim = (_vm->gameFlags().lang == Common::KO_KOR) ? limKor : limDef;

	Screen::FontId curFont = _screen->setFont(_vm->gameFlags().lang == Common::Language::ZH_TWN && _vm->gameFlags().gameID == GI_LOL ? Screen::FID_CHINESE_FNT :
						  _vm->gameFlags().lang == Common::KO_KOR ? Screen::FID_KOREAN_FNT : Screen::FID_8_FNT);
	_screen->_charSpacing = -2;
	int textWidth = _screen->getTextWidth(p);
	_screen->_charSpacing = 0;
	if (textWidth > lim[0]) {
		if (textWidth > lim[1]) {
			int count = getCharLength(p, textWidth / 3);
			int offs = dropCRIntoString(p, count);
			p += count + offs;
			_screen->_charSpacing = -2;
			textWidth = _screen->getTextWidth(p);
			_screen->_charSpacing = 0;
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
	int currentLine = 0;
	int pos = 0;
	while (*str) {
		if (*str == _lineBreakChar) {
			assert(currentLine < TALK_SUBSTRING_NUM);
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
			++currentLine;
			pos = 0;
		} else {
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = *str;
			++pos;
			if (_vm->game() == GI_KYRA2 && _vm->gameFlags().lang == Common::ZH_TWN && pos == 32) {
				_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
				++currentLine;
				pos = 0;
			} else if (pos >= TALK_SUBSTRING_LEN - 2) {
				pos = TALK_SUBSTRING_LEN - 2;
			}
		}
		++str;
	}
	_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
	return currentLine + 1;
}

int TextDisplayer::getWidestLineWidth(int linesCount) {
	int maxWidth = 0;
	_screen->_charSpacing = -2;
	for (int l = 0; l < linesCount; ++l) {
		int w = _screen->getTextWidth(&_talkSubstrings[l * TALK_SUBSTRING_LEN]);
		if (maxWidth < w) {
			maxWidth = w;
		}
	}
	_screen->_charSpacing = 0;
	return maxWidth;
}

void TextDisplayer::calcWidestLineBounds(int &x1, int &x2, int w, int cx) {
	int margin = (_vm->game() == GI_KYRA2 && _vm->gameFlags().lang == Common::ZH_TWN) ? 8 : 12;
	x1 = cx - w / 2;
	if (x1 + w >= Screen::SCREEN_W - margin) {
		x1 = Screen::SCREEN_W - margin - w - 1;
	} else if (x1 < margin) {
		x1 = margin;
	}
	x2 = x1 + w + 1;
}

void TextDisplayer::restoreTalkTextMessageBkgd(int srcPage, int dstPage) {
	if (_talkMessagePrinted) {
		_talkMessagePrinted = false;
		_screen->copyRegion(_talkCoords.x, _talkCoords.y, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, srcPage, dstPage, Screen::CR_NO_P_CHECK);
	}
}

void TextDisplayer::printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage) {
	char *str = preprocessString(text);
	int lineCount = buildMessageSubstrings(str);
	// For Chinese we call this before recalculating the line count
	int w = getWidestLineWidth(lineCount);
	int marginTop = 0;

	if (_vm->gameFlags().lang == Common::ZH_TWN) {
		lineCount = (strlen(str) + 31) >> 5;
		marginTop = 10;
		w = MIN<int>(w, 302);
	}

	int top = y - (lineCount * _screen->getFontHeight() + (lineCount - 1) * _screen->_lineSpacing) - _langExtraSpacing;
	if (top < marginTop)
		top = marginTop;

	_talkMessageY = top;
	_talkMessageH = (lineCount * _screen->getFontHeight() + (lineCount - 1) * _screen->_lineSpacing) + _langExtraSpacing;

	int x1 = 12;
	int x2 = Screen::SCREEN_W - 12;
	if (_vm->gameFlags().lang != Common::ZH_TWN || lineCount == 1)
		calcWidestLineBounds(x1, x2, w, x);

	_talkCoords.x = x1;
	_talkCoords.w = w + 2;
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkCoords.y, _talkCoords.w, _talkMessageH, srcPage, dstPage, Screen::CR_NO_P_CHECK);
	int curPage = _screen->_curPage;
	_screen->_curPage = srcPage;

	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		setTextColor(color);

	if (_vm->gameFlags().lang == Common::ZH_TWN && lineCount > 1) {
		// The Chinese version leaves the wrapping to the default font handling
		printText(_talkSubstrings, 12, top, color, 0xC, 0xC);
	} else {
		for (int i = 0; i < lineCount; ++i) {
			top = i * (_screen->getFontHeight() + _screen->_lineSpacing) + _talkMessageY;
			char *msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
			int left = getCenterStringX(msg, x1, x2);
			printText(msg, left, top, color, 0xC, _vm->gameFlags().lang == Common::ZH_TWN ? 0xC : 0);
		}
	}
	_screen->_curPage = curPage;
	_talkMessagePrinted = true;
}

void TextDisplayer::printText(const Common::String &str, int x, int y, uint8 c0, uint8 c1, uint8 c2) {
	Common::String revBuffer;
	const char *tmp = str.c_str();
	if (_vm->gameFlags().lang == Common::HE_ISR) {
		for (int i = str.size() - 1; i >= 0; --i)
			revBuffer += str[i];
		tmp = revBuffer.c_str();
	}
	uint8 colorMap[] = { 0, 15, 12, 12 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	_screen->_charSpacing = -2;
	int ls = _screen->_lineSpacing;
	_screen->_lineSpacing = _langExtraSpacing >> 1;
	_screen->printText(tmp, x, y, c0, c2);
	_screen->_charSpacing = 0;
	_screen->_lineSpacing = ls;
}

void TextDisplayer::printCharacterText(const char *text, int8 charNum, int charX) {
	int top, left, w, x;
	char *msg;

	text = preprocessString(text);
	int lineCount = buildMessageSubstrings(text);
	// For Chinese we call this before recalculating the line count
	w = getWidestLineWidth(lineCount);

	if (_vm->gameFlags().lang == Common::ZH_TWN) {
		lineCount = (strlen(text) + 31) >> 5;
		w = MIN<int>(w, 302);
	}

	int x1 = 12;
	int x2 = Screen::SCREEN_W - 12;
	if (_vm->gameFlags().lang != Common::ZH_TWN || lineCount == 1) {
		x = charX;
		calcWidestLineBounds(x1, x2, w, x);
	}

	uint8 color = 0;
	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		const uint8 colorTable[] = { 0x1F, 0x1B, 0xC9, 0x80, 0x1E, 0x81, 0x11, 0xD8, 0x55, 0x3A, 0x3A };
		color = colorTable[charNum];

		setTextColor(color);
	} else {
		const uint8 colorTable[] = { 0x0F, 0x09, 0xC9, 0x80, 0x05, 0x81, 0x0E, 0xD8, 0x55, 0x3A, 0x3A };
		color = colorTable[charNum];
	}

	if (_vm->gameFlags().lang == Common::ZH_TWN && lineCount > 1) {
		// The Chinese version leaves the wrapping to the default font handling
		printText(_talkSubstrings, 12, _talkMessageY, color, 0xC, 0xC);
		return;
	}

	for (int i = 0; i < lineCount; ++i) {
		top = i * (_screen->getFontHeight() + _screen->_lineSpacing) + _talkMessageY;
		msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, _vm->gameFlags().lang == Common::ZH_TWN ? 0xC : 0);
	}
}

void TextDisplayer::setTextColor(uint8 color) {
	byte r, g, b;

	switch (color) {
	case 4:
		// 0x09E
		r = 0;
		g = 37;
		b = 58;
		break;

	case 5:
		// 0xFF5
		r = 63;
		g = 63;
		b = 21;
		break;

	case 27:
		// 0x5FF
		r = 21;
		g = 63;
		b = 63;
		break;

	case 34:
		// 0x8E5
		r = 33;
		g = 58;
		b = 21;
		break;

	case 58:
		// 0x9FB
		r = 37;
		g = 63;
		b = 46;
		break;

	case 85:
		// 0x7CF
		r = 29;
		g = 50;
		b = 63;
		break;

	case 114:
	case 117:
		// 0xFAF
		r = 63;
		g = 42;
		b = 63;
		break;

	case 128:
	case 129:
		// 0xFCC
		r = 63;
		g = 50;
		b = 50;
		break;

	case 201:
		// 0xFD8
		r = 63;
		g = 54;
		b = 33;
		break;

	case 216:
		// 0xFC6
		r = 63;
		g = 50;
		b = 25;
		break;

	default:
		// 0xEEE
		r = 58;
		g = 58;
		b = 58;
	}

	_screen->setPaletteIndex(0x10, r, g, b);
}

} // End of namespace Kyra
