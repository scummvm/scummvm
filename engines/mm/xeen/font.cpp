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

#include "common/endian.h"
#include "mm/xeen/font.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

namespace {
const int kBig5Width = 16;
const int kBig5Height = 14;
}

const byte *FontData::_fontData;
Common::Point *FontData::_fontWritePos;
byte FontData::_textColors[4];
byte FontData::_bgColor;
bool FontData::_fontReduced;
Justify FontData::_fontJustify;
Graphics::Big5Font *FontData::_big5Font;
	
FontSurface::FontSurface() : XSurface(), _msgWraps(false), _displayString(nullptr),
		_writePos(*FontData::_fontWritePos) {
	setTextColor(0);

	lang = g_vm->getLanguage();
	if (Common::RU_RUS == lang) {
		_fntEnOffset            = 0x0000;
		_fntNonEnOffset         = 0x0800;
		_fntEnReducedOffset     = 0x1000;
		_fntNonEnReducedOffset  = 0x1800;
		_fntEnWOffset           = 0x2000;
		_fntNonEnWOffset        = 0x2080;
		_fntEnReducedWOffset    = 0x2100;
		_fntNonEnReducedWOffset = 0x2180;
	} else {
		_fntEnOffset            = 0x0000;
		_fntEnReducedOffset     = 0x0800;
		_fntEnWOffset           = 0x1000;
		_fntEnReducedWOffset    = 0x1080;
	}
	_isBig5 = Common::ZH_TWN == lang;
}

FontSurface::FontSurface(int wv, int hv) : XSurface(wv, hv),
		_msgWraps(false), _displayString(nullptr), _writePos(*FontData::_fontWritePos) {
	create(w, h);
	setTextColor(0);

	lang = g_vm->getLanguage();
	if (Common::RU_RUS == lang) {
		_fntEnOffset            = 0x0000;
		_fntNonEnOffset         = 0x0800;
		_fntEnReducedOffset     = 0x1000;
		_fntNonEnReducedOffset  = 0x1800;
		_fntEnWOffset           = 0x2000;
		_fntNonEnWOffset        = 0x2080;
		_fntEnReducedWOffset    = 0x2100;
		_fntNonEnReducedWOffset = 0x2180;
	} else {
		_fntEnOffset            = 0x0000;
		_fntEnReducedOffset     = 0x0800;
		_fntEnWOffset           = 0x1000;
		_fntEnReducedWOffset    = 0x1080;
	}
	_isBig5 = Common::ZH_TWN == lang;
}

void FontSurface::writeSymbol(int symbolId) {
	const byte *srcP = &Res.SYMBOLS[symbolId][0];

	for (int yp = 0; yp < FONT_HEIGHT; ++yp) {
		byte *destP = (byte *)getBasePtr(_writePos.x, _writePos.y + yp);

		for (int xp = 0; xp < FONT_WIDTH; ++xp, ++destP) {
			byte b = *srcP++;
			if (b)
				*destP = b;
		}
	}

	_writePos.x += 8;
}

bool FontSurface::isSpace(char c) {
	if (Common::RU_RUS == lang) 
		return c == ' ';
	return (c & 0x7f) == ' ';
}

const char *FontSurface::writeString(const Common::String &s, const Common::Rect &bounds) {
	_displayString = s.c_str();
	assert(_fontData);

	for (;;) {
		const char *msgStartP = _displayString;
		_msgWraps = false;

		// Get the size of the string that can be displayed on the line
		int xp = _fontJustify == JUSTIFY_CENTER ? bounds.left : _writePos.x;
		while (!getNextCharWidth(xp)) {
			if (xp >= bounds.right) {
				--_displayString;
				_msgWraps = true;
				break;
			}
		}

		// Get the end point of the text that can be displayed
		const char *displayEnd = _displayString;
		_displayString = msgStartP;

		if (_msgWraps && _fontJustify != JUSTIFY_RIGHT && xp >= bounds.right) {
			// Need to handle justification of text
			// First, move backwards to find the end of the previous word
			// for a convenient point to break the line at
			const char *endP = displayEnd;
			while (endP > _displayString && !isSpace(*endP))
				--endP;

			if (endP == _displayString) {
				// There was no word breaks at all in the string
				// Since the loop below includes displayEnd, we need to
				// subtract 1 to compensate and another time to delete last
				// char.
				displayEnd -= 2;
				if (_fontJustify == JUSTIFY_NONE && _writePos.x != bounds.left) {
					// Move to the next line
					if (!newLine(bounds))
						continue;
					// Ran out of space to display string
					break;
				}
			} else {
				// Found word break, find end of previous word
				while (endP > _displayString && !isSpace(*endP))
					--endP;

				displayEnd = endP;
			}
		}

		// Justification adjustment
		if (_fontJustify != JUSTIFY_NONE) {
			// Figure out the width of the selected portion of the string
			int totalWidth = 0;
			while (!getNextCharWidth(totalWidth)) {
				if (_displayString > displayEnd) {
					if (*displayEnd == ' ') {
						// Don't include any ending space as part of the total
						totalWidth -= _fontReduced ? 4 : 5;
					}
					break;
				}
			}

			// Reset starting position back to the start of the string portion
			_displayString = msgStartP;

			if (_fontJustify == JUSTIFY_RIGHT) {
				// Right aligned
				if (_writePos.x == bounds.left)
					_writePos.x = bounds.right;
				_writePos.x -= totalWidth + 1;
			} else {
				// Center aligned
				if (_writePos.x == bounds.left)
					_writePos.x = (bounds.left + bounds.right + 1 - totalWidth) / 2;
				else
					_writePos.x = (_writePos.x * 2 - totalWidth) / 2;
 			}
		}

		// Main character display loop
		while (_displayString <= displayEnd) {
			uint16_t c = getNextChar();

			if (c == ' ') {
				_writePos.x += _fontReduced ? 3 : 4;
			} else if (c == '\r') {
				fillRect(bounds, _bgColor);
				addDirtyRect(bounds);
				_writePos = Common::Point(bounds.left, bounds.top);
			} else if (c == 1) {
				// Turn off reduced font mode
				_fontReduced = false;
			} else if (c == 2) {
				// Turn on reduced font mode
				_fontReduced = true;
			} else if (c == 3) {
				// Justify text
				c = getNextChar();
				if (c == 'r')
					_fontJustify = JUSTIFY_RIGHT;
				else if (c == 'c')
					_fontJustify = JUSTIFY_CENTER;
				else
					_fontJustify = JUSTIFY_NONE;
			} else if (c == 4) {
				// Draw an empty box of a given width
				int wv = fontAtoi();
				Common::Point pt = _writePos;
				if (_fontJustify == JUSTIFY_RIGHT)
					pt.x -= wv;

				Common::Rect r(pt.x, pt.y, pt.x + wv, pt.y + (_fontReduced ? 9 : 10));
				fillRect(r, _bgColor);
			} else if (c == 5) {
				continue;
			} else if (c == 6) {
				// Non-breakable space
				writeChar(' ', bounds);
			} else if (c == 7) {
				// Set text background color
				int bgColor = fontAtoi();
				_bgColor = (bgColor < 0 || bgColor > 255) ? DEFAULT_BG_COLOR : bgColor;
			} else if (c == 8) {
				// Draw a character outline
				c = getNextChar();
				if (c == ' ') {
					c = '\0';
					_writePos.x -= 3;
				} else {
					if (c == 6)
						c = ' ';
					int offset_charW = c & 0x80 ?
						(_fontReduced ? _fntNonEnReducedWOffset : _fntNonEnWOffset) + (int)(c & 0x7F) :
						(_fontReduced ? _fntEnReducedWOffset : _fntEnWOffset) + (int)c;
					byte charSize = _fontData[offset_charW];

					_writePos.x -= charSize;
				}

				if (_writePos.x < bounds.left)
					_writePos.x = bounds.left;

				if (c) {
					int oldX = _writePos.x;
					byte oldColor[4];
					Common::copy(&_textColors[0], &_textColors[4], &oldColor[0]);

					_textColors[1] = _textColors[2] = _textColors[3] = _bgColor;
					writeChar(c, bounds);

					Common::copy(&oldColor[0], &oldColor[4], &_textColors[0]);
					_writePos.x = oldX;
				}
			} else if (c == 9) {
				// Skip x position
				int xAmount = fontAtoi();
				_writePos.x = MIN(bounds.left + xAmount, (int)bounds.right);
			} else if (c == 10) {
				// Newline
				if (newLine(bounds))
					return _displayString;
			} else if (c == 11) {
				// Set y position
				int yp = fontAtoi();
				_writePos.y = MIN(bounds.top + yp, (int)bounds.bottom);
			} else if (c == 12) {
				// Set text colors
				int idx = fontAtoi(2);
				if (idx < 0)
					idx = 0;
				setTextColor(idx);
			} else if (Common::RU_RUS == lang && (c & 0x80)) {
				writeChar(c, bounds);
			} else if (c < ' ') {
				// End of string or invalid command
				_displayString = nullptr;
				break;
			} else {
				// Standard character - write it out
				writeChar(c, bounds);
			}
		}

		if (!_displayString)
			break;
		if ( _displayString > displayEnd && _fontJustify != JUSTIFY_RIGHT && _msgWraps
				&& newLine(bounds))
			break;
	}

	return _displayString;
}

void FontSurface::writeCharacter(uint16_t c, const Common::Rect &clipRect) {
	Justify justify = _fontJustify;
	_fontJustify = JUSTIFY_NONE;
	if (_isBig5 && c >= 0x100)
		writeString(Common::String::format("%c%c", c >> 8, c & 0xff), clipRect);
	else
		writeString(Common::String::format("%c", c), clipRect);
	_fontJustify = justify;
}

uint16_t FontSurface::getNextChar() {
	if (_isBig5) {
		uint8_t lead = *_displayString++;
		if (!(lead & 0x80))
			return lead;
		return (lead << 8) | (*_displayString++ & 0xff);
	} else if (Common::RU_RUS == lang)
		return *_displayString++ & 0xff;
	else
		return *_displayString++ & 0x7f;
}

bool FontSurface::getNextCharWidth(int &total) {
	uint16_t c = getNextChar();

	if (_isBig5 && c > 0xff) {
		total += kBig5Width;
		return false;
	} if (c > ' ') {
		total += _fontData[(_fontReduced ? _fntEnReducedWOffset : _fntEnWOffset) + (int)c];
		return false;
	} else if (c == ' ') {
		total += 4;
		return false;
	} else if (c == 8) {
		c = getNextChar();
		if (c == ' ') {
			total -= 2;
			return false;
		} else {
			_displayString -= 2;
			return true;
		}
	} else if (c == 12) {
		c = getNextChar();
		if (c != 'd')
			getNextChar();
		return false;
	} else if (Common::RU_RUS == lang && (c & 0x80)) {
		total += _fontData[(_fontReduced ? _fntNonEnReducedWOffset : _fntNonEnWOffset) + (int)(c & 0x7F)];
		return false;
	} else {
		--_displayString;
		return true;
	}
}

bool FontSurface::newLine(const Common::Rect &bounds) {
	// Move past any spaces currently being pointed to
	while (isSpace(*_displayString))
		++_displayString;

	_msgWraps = false;
	_writePos.x = bounds.left;

	int hv = _isBig5 && _big5Font ? _big5Font->getFontHeight() + 1 : (_fontReduced ? 9 : 10);
	_writePos.y += hv;

	return ((_writePos.y + hv - 1) > bounds.bottom);
}

int FontSurface::fontAtoi(int len) {
	int total = 0;
	for (int i = 0; i < len; ++i) {
		uint16_t c = getNextChar();
		if (c == ' ')
			c = '0';

		int digit = c - '0';
		if (digit < 0 || digit > 9)
			return -1;

		total = total * 10 + digit;
	}

	return total;
}

void FontSurface::setTextColor(int idx) {
	const byte *colP = (g_vm->_mode == MODE_STARTUP) ? &Res.TEXT_COLORS_STARTUP[idx][0] : &Res.TEXT_COLORS[idx][0];
	Common::copy(colP, colP + 4, &_textColors[0]);
}

void FontSurface::writeChar(uint16_t c, const Common::Rect &clipRect) {
	// Get y position, handling kerning
	int y = _writePos.y;
	if (_isBig5 && c > 0xff) {
		_big5Font->drawBig5Char(surfacePtr(), c, _writePos, _textColors[0]);
		_writePos.x++;
		_big5Font->drawBig5Char(surfacePtr(), c, _writePos, _textColors[3]);
		_writePos.x += kBig5Width - 1;
		addDirtyRect(Common::Rect(_writePos.x, _writePos.y, _writePos.x + kBig5Width,
					  _writePos.y + kBig5Height));

		return;
	}
	if (c == 'g' || c == 'p' || c == 'q' || c == 'y')
		++y;
	if (_isBig5 && _big5Font) {
		y += (_big5Font->getFontHeight() - FONT_HEIGHT) / 2;
	}
	int yStart = y;

	// Get pointers into font data and surface to write pixels to
	int offset_charData;
	int offset_charW;
	if (Common::RU_RUS == lang && (c & 0x80)) {
		offset_charData = (_fontReduced ? _fntNonEnReducedOffset : _fntNonEnOffset) + (int)(c & 0x7F) * 16;
		offset_charW = (_fontReduced ? _fntNonEnReducedWOffset : _fntNonEnWOffset) + (int)(c & 0x7F);
	} else {
		offset_charData = (_fontReduced ? _fntEnReducedOffset : _fntEnOffset) + (int)c * 16;
		offset_charW = (_fontReduced ? _fntEnReducedWOffset : _fntEnWOffset) + (int)c;
	}
	const byte *srcP = &_fontData[offset_charData];

	for (int yp = 0; yp < FONT_HEIGHT; ++yp, ++y) {
		uint16 lineData = READ_LE_UINT16(srcP); srcP += 2;
		byte *destP = (byte *)getBasePtr(_writePos.x, y);

		// Ignore line if it's outside the clipping rect
		if (y < clipRect.top || y >= clipRect.bottom)
			continue;
		const byte *lineStart = (const byte *)getBasePtr(clipRect.left, y);
		const byte *lineEnd = (const byte *)getBasePtr(clipRect.right, y);

		for (int xp = 0; xp < FONT_WIDTH; ++xp, ++destP) {
			int colIndex = lineData & 3;
			lineData >>= 2;

			if (colIndex && destP >= lineStart && destP < lineEnd)
				*destP = _textColors[colIndex];
		}
	}

	addDirtyRect(Common::Rect(_writePos.x, yStart, _writePos.x + FONT_WIDTH,
		yStart + FONT_HEIGHT));
	_writePos.x += _fontData[offset_charW];
}

} // End of namespace Xeen
} // End of namespace MM
