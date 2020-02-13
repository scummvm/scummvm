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

#include "hopkins/font.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace Hopkins {

FontManager::FontManager(HopkinsEngine *vm) {
	_vm = vm;
	clearAll();
}

FontManager::~FontManager() {
	_vm->_globals->freeMemory(_font);
	_vm->_globals->freeMemory(_zoneText);
}

void FontManager::loadZoneText() {
	switch (_vm->_globals->_language) {
	case LANG_EN:
		_zoneText = _vm->_fileIO->loadFile("ZONEAN.TXT");
		break;
	case LANG_FR:
		_zoneText = _vm->_fileIO->loadFile("ZONE01.TXT");
		break;
	case LANG_SP:
		_zoneText = _vm->_fileIO->loadFile("ZONEES.TXT");
		break;
	default:
		break;
	}
}

void FontManager::clearAll() {
	_font = NULL;
	_fontFixedHeight = 0;
	_fontFixedWidth = 0;

	for (int idx = 0; idx < 12; ++idx) {
		Common::fill((byte *)&_text[idx], (byte *)&_text[idx] + sizeof(TxtItem), 0);

		_textList[idx]._enabledFl = false;
		_textList[idx]._height = 0;
		_textList[idx]._width = 0;
		_textList[idx]._pos.x = 0;
		_textList[idx]._pos.y = 0;
	}

	for (int idx = 0; idx < 21; idx++)
		_textSortArray[idx] = 0;

	_oldName = Common::String("");
	_indexName = Common::String("");

	for (int idx = 0; idx < 4048; idx++)
		_index[idx] = 0;

	_tempText = NULL;
	_zoneText = NULL;

	_boxWidth = 240;
}

void FontManager::initData() {
	_font = _vm->_fileIO->loadFile("FONTE3.SPR");
	_fontFixedWidth = 12;
	_fontFixedHeight = 21;
	loadZoneText();
}
/**
 * Display Text
 */
void FontManager::showText(int idx) {
	if ((idx - 5) > MAX_TEXT)
		error("Attempted to display text > MAX_TEXT.");

	TxtItem &txt = _text[idx - 5];
	txt._textOnFl = true;
	txt._textLoadedFl = false;

	txt._textBlock = _vm->_globals->freeMemory(txt._textBlock);
}

/**
 * Hide text
 */
void FontManager::hideText(int idx) {
	if ((idx - 5) > MAX_TEXT)
			error("Attempted to display text > MAX_TEXT.");

	TxtItem &txt = _text[idx - 5];
	txt._textOnFl = false;
	txt._textLoadedFl = false;
	txt._textBlock = _vm->_globals->freeMemory(txt._textBlock);
}

/**
 * Set Text Color
 */
void FontManager::setTextColor(int idx, byte colByte) {
	_text[idx - 5]._color = colByte;
}

/**
 * Set Text Optimal Color
 */
void FontManager::setOptimalColor(int idx1, int idx2, int idx3, int idx4) {
	setTextColor(idx1, 255);
	setTextColor(idx2, 255);
	setTextColor(idx3, 255);
	setTextColor(idx4, 253);
}

/**
 * Init text structure
 */
void FontManager::initTextBuffers(int idx, int messageId, const Common::String &filename, int xp, int yp, int textType, int length, int color) {
	assert(idx - 5 >= 0 && (idx - 5) <= MAX_TEXT);

	TxtItem &txt = _text[idx - 5];
	txt._textOnFl = false;
	txt._filename = filename;
	txt._pos.x = xp;
	txt._pos.y = yp;
	txt._messageId = messageId;
	txt._textType = textType;
	txt._length = length;
	txt._color = color;
}

// Box
void FontManager::box(int idx, int messageId, const Common::String &filename, int xp, int yp) {
	int textPosX = xp;
	if (idx < 0)
		error("Bad number for text");
	_fontFixedWidth = 11;

	_boxWidth = 11 * _text[idx]._length;
	if (_text[idx]._textLoadedFl) {
		int textType = _text[idx]._textType;
		if (textType != 6 && textType != 1 && textType != 3 && textType != 5) {
			int yCurrent = yp + 5;
			for (int lineNum = 0; lineNum < _text[idx]._lineCount; ++lineNum) {
				displayText(xp + 5, yCurrent, _text[idx]._lines[lineNum], _text[idx]._color);
				yCurrent += _fontFixedHeight + 1;
			}
		} else {
			int height = _text[idx]._height;
			int width = _text[idx]._width;
			_vm->_graphicsMan->restoreSurfaceRect(
				_vm->_graphicsMan->_frontBuffer,
				_text[idx]._textBlock,
				xp,
				yp,
				_text[idx]._width,
				_text[idx]._height);
			_vm->_graphicsMan->addDirtyRect(xp, yp, xp + width, yp + height);
		}
	} else {
		int lineCount = 0;
		for (int i = 0; i <= 19; i++)
			_textSortArray[i] = 0;

		_text[idx]._textLoadedFl = true;
		Common::String file = filename;
		if (strncmp(file.c_str(), _oldName.c_str(), strlen(file.c_str())) != 0) {
			// Starting to access a new file, so read in the index file for the file
			_oldName = file;
			_indexName = Common::String(file.c_str(), file.size() - 3);
			_indexName += "IND";

			Common::File f;
			if (!f.open(_indexName))
				error("Error opening file - %s", _indexName.c_str());
			int filesize = f.size();
			for (int i = 0; i < (filesize / 4); ++i)
				_index[i] = f.readUint32LE();
			f.close();
		}
		int bufSize;
		if (filename[0] != 'Z' || filename[1] != 'O') {
			Common::File f;
			if (!f.open(file))
				error("Error opening file - %s", _indexName.c_str());

			bufSize = 2048;
			f.seek(_index[messageId]);

			_tempText = _vm->_globals->allocMemory(2058);
			if (_tempText == NULL)
				error("Error allocating text");

			Common::fill(&_tempText[0], &_tempText[2058], 0);
			f.read(_tempText, 2048);
			f.close();
		} else {
			bufSize = 100;
			_tempText = _vm->_globals->allocMemory(110);
			Common::fill(&_tempText[0], &_tempText[110], 0);
			memcpy(_tempText, _zoneText + _index[messageId], 96);
			WRITE_LE_UINT16((uint16 *)_tempText + 48, READ_LE_INT16(_zoneText + _index[messageId] + 96));
		}
		byte *curTempTextPtr = _tempText;
		for (int i = 0; i < bufSize; i++) {
			byte curChar = *curTempTextPtr;
			if ((byte)(*curTempTextPtr + 46) > 27) {
				if ((byte)(curChar + 80) > 27) {
					if ((byte)(curChar - 65) <= 25 || (byte)(curChar - 97) <= 25)
						curChar = 32;
				} else {
					curChar -= 79;
				}
			} else {
				curChar += 111;
			}
			*curTempTextPtr = curChar;
			curTempTextPtr++;
		};

		int textLength;
		for (textLength = 0; textLength < bufSize; textLength++) {
			byte curChar = _tempText[textLength];
			if (curChar == '\r' || curChar == '\n') {
				_tempText[textLength] = 0;
				if (!_text[idx]._length)
					break;
			}
		}

		if (bufSize && bufSize > textLength) {
			_text[idx]._length = textLength;
			_boxWidth = 0;

			for (int curStrIdx = 0; curStrIdx < textLength + 1; curStrIdx++) {
				byte curChar = _tempText[curStrIdx];
				if (curChar <= 31)
					curChar = ' ';
				_boxWidth += _vm->_objectsMan->getWidth(_font, curChar - 32);
			}

			_boxWidth += 2;
			_text[idx]._pos.x = 320 - abs(_boxWidth / 2);
			textPosX = _vm->_events->_startPos.x + _text[idx]._pos.x;
			lineCount = 1;
			_text[idx]._lines[0] = Common::String((const char *)_tempText, textLength);
		} else {
			if (!_boxWidth)
				_boxWidth = 240;
			int tempTextIdx = 0;
			int lineSize;
			byte curChar;
			do {
				int curLineSize = 0;
				int ptrb = _boxWidth - 4;
				for (;;) {
					lineSize = curLineSize;
					do {
						curChar = _tempText[tempTextIdx + curLineSize++];
					} while (curChar != ' ' && curChar != '%');
					if (curLineSize >= ptrb / _fontFixedWidth) {
						if (curChar == '%')
							curChar = ' ';
						break;
					}
					if (curChar == '%') {
						lineSize = curLineSize;
						break;
					}
				}

				// WORKAROUND: Perhaps due to the usage of ScummVM strings here, recalculate what the
				// actual length of the line to be copied will be. Otherwise, you can see artifacts,
				// such as a single character beyond the end of string NULL.
				int actualSize = 0;
				while (actualSize < lineSize && _tempText[tempTextIdx + actualSize])
					++actualSize;

				_text[idx]._lines[lineCount] = Common::String((const char *)_tempText + tempTextIdx, actualSize);
				_textSortArray[lineCount++] = lineSize;

				tempTextIdx += lineSize;
			} while (curChar != '%');

			for (int i = 0; i <= 19; i++) {
				if (_textSortArray[i] <= 0) {
					_textSortArray[i] = 0;
				} else {
					int ptrc = 0;
					for (int curIdx = 0; curIdx < _textSortArray[i] - 1; curIdx++) {
						Common::String &line = _text[idx]._lines[i];
						byte curChar2 = (curIdx >= (int)line.size()) ? '\0' : line.c_str()[curIdx];
						if (curChar2 <= 31)
							curChar2 = ' ';
						ptrc += _vm->_objectsMan->getWidth(_font, (byte)curChar2 - 32);
					}
					_textSortArray[i] = ptrc;
				}
			}
			for (int i = 0; i <= 19; i++) {
				for (int j = i + 1; j != i; j = (j + 1) % 20) {
					if (_textSortArray[i] < _textSortArray[j])
						_textSortArray[i] = 0;
				}
			};

			for (int i = 0; i <= 19; i++) {
				if (_textSortArray[i])
					_boxWidth = _textSortArray[i];
			}

			if ((_text[idx]._textType < 2) || (_text[idx]._textType > 3)) {
				int i;
				for (i = xp - _vm->_events->_startPos.x; _boxWidth + i > 638 && i > -2 && _text[idx]._textType; i -= 2)
					;
				_text[idx]._pos.x = i;
				textPosX = _vm->_events->_startPos.x + i;
			} else {
				_text[idx]._pos.x = textPosX;
			}
		}
		int posX = textPosX;
		int posY = yp;
		int saveWidth = _boxWidth + 10;
		int saveHeight = (_fontFixedHeight + 1) * lineCount + 12;
		if (_text[idx]._textType == 6) {
			_text[idx]._pos.x = 315 - abs(saveWidth / 2);
			textPosX = posX = _vm->_events->_startPos.x + _text[idx]._pos.x;
			_text[idx]._pos.y = posY = 50;
		}
		int textType = _text[idx]._textType;
		if (textType == 1 || textType == 3 || textType == 5 || textType == 6) {
			int size = saveHeight * saveWidth;
			byte *ptrd = _vm->_globals->allocMemory(size);
			if (ptrd == NULL)
				error("Cutting a block for text box (%d)", size);

			_vm->_graphicsMan->copySurfaceRect(_vm->_graphicsMan->_frontBuffer, ptrd, posX, posY, saveWidth, saveHeight);
			_vm->_graphicsMan->fillSurface(ptrd, _vm->_graphicsMan->_colorTable, size);
			_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, ptrd, posX, posY, saveWidth, saveHeight);
			_vm->_globals->freeMemory(ptrd);

			_vm->_graphicsMan->drawHorizontalLine(_vm->_graphicsMan->_frontBuffer, posX, posY, saveWidth, (byte)-2);
			_vm->_graphicsMan->drawHorizontalLine(_vm->_graphicsMan->_frontBuffer, posX, saveHeight + posY, saveWidth, (byte)-2);
			_vm->_graphicsMan->drawVerticalLine(_vm->_graphicsMan->_frontBuffer, posX, posY, saveHeight, (byte)-2);
			_vm->_graphicsMan->drawVerticalLine(_vm->_graphicsMan->_frontBuffer, saveWidth + posX, posY, saveHeight, (byte)-2);
		}
		_text[idx]._lineCount = lineCount;
		int textPosY = posY + 5;

		for (int lineNum = 0; lineNum < lineCount; ++lineNum) {
			displayText(textPosX + 5, textPosY, _text[idx]._lines[lineNum], _text[idx]._color);
			textPosY += _fontFixedHeight + 1;
		}

		int blockWidth = saveWidth + 1;
		int blockHeight = saveHeight + 1;

		_text[idx]._width = blockWidth;
		_text[idx]._height = blockHeight;
		textType = _text[idx]._textType;
		if (textType == 6 || textType == 1 || textType == 3 || textType == 5) {
			_text[idx]._textBlock = _vm->_globals->freeMemory(_text[idx]._textBlock);
			int blockSize = blockHeight * blockWidth;
			byte *ptre = _vm->_globals->allocMemory(blockSize + 20);
			if (ptre == NULL)
				error("Cutting a block for text box (%d)", blockSize);

			_text[idx]._textBlock = ptre;
			_text[idx]._width = blockWidth;
			_text[idx]._height = blockHeight;
			_vm->_graphicsMan->copySurfaceRect(_vm->_graphicsMan->_frontBuffer, _text[idx]._textBlock, posX, posY, _text[idx]._width, blockHeight);
		}
		_tempText = _vm->_globals->freeMemory(_tempText);
	}
}

/**
 * Directly display text (using a VESA segment)
 */
void FontManager::displayTextVesa(int xp, int yp, const Common::String &message, int col) {
	int charIndex;
	int currentX = xp;

	const char *srcP = message.c_str();
	for (;;) {
		byte currChar = *srcP++;
		if (!currChar)
			break;
		if (currChar >= 32) {
			charIndex = currChar - 32;
			_vm->_graphicsMan->displayFont(_vm->_graphicsMan->_frontBuffer, _font, currentX, yp, currChar - 32, col);
			currentX += _vm->_objectsMan->getWidth(_font, charIndex);
		}
	}

	_vm->_graphicsMan->addDirtyRect(xp, yp, currentX, yp + 12);
}

/**
 * Directly display text
 */
void FontManager::displayText(int xp, int yp, const Common::String &message, int col) {
	for (uint idx = 0; idx < message.size(); ++idx) {
		byte currentChar = (byte)message[idx];

		if (currentChar > 31) {
			int characterIndex = currentChar - 32;
			_vm->_graphicsMan->displayFont(_vm->_graphicsMan->_frontBuffer, _font, xp, yp, characterIndex, col);
			_vm->_graphicsMan->addDirtyRect(xp, yp, xp + _vm->_objectsMan->getWidth(_font, characterIndex) + 1, yp + _vm->_objectsMan->getHeight(_font, characterIndex) + 1);
			xp += _vm->_objectsMan->getWidth(_font, characterIndex);
		}
	}
}

/**
 * Compute character width and render text using variable width fonts
 */
void FontManager::renderTextDisplay(int xp, int yp, const Common::String &msg, int col) {
	const char *srcP = msg.c_str();
	int charEndPosX = xp;
	int fontCol = col;
	byte curChar = *srcP++;
	while (curChar) {
		if (curChar == '&') {
			fontCol = 2;
			curChar = *srcP++;
		}
		if (curChar == '$') {
			fontCol = 4;
			curChar = *srcP++;
		}
		if (!curChar)
			break;
		if (curChar >= 32) {
			byte printChar = curChar - 32;
			_vm->_graphicsMan->displayFont(_vm->_graphicsMan->_frontBuffer, _font, charEndPosX, yp, printChar, fontCol);

			// UGLY HACK: For some obscure reason, the BeOS and OS/2 versions use another font file, which doesn't have variable width.
			// All the fonts have a length of 9, which results in completely broken text in the computer.
			// This horrible workaround fixes the English versions of the game. So far, no other languages are known for those platforms.
			// Just in case, all the accentuated characters are handled properly, which *should* be OK for the other languages too.
			int charWidth;
			if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
				if ((curChar >= 'A' && curChar <= 'Z') || (curChar >= 'a' && curChar <= 'z' && curChar != 'm' && curChar != 'w') || (curChar >= '0' && curChar <= '9') || curChar == '*' || (curChar >= 128 && curChar <= 168))
					charWidth = _vm->_objectsMan->getWidth(_font, printChar) - 1;
				else if (curChar == 'm' || curChar == 'w')
					charWidth = _vm->_objectsMan->getWidth(_font, printChar);
				else
					charWidth = 6;
			} else
				charWidth = _vm->_objectsMan->getWidth(_font, printChar);

			int charStartPosX = charEndPosX;
			charEndPosX += charWidth;
			_vm->_graphicsMan->addDirtyRect(charStartPosX, yp, charEndPosX, yp + 12);
			if (_vm->_events->_escKeyFl) {
				_vm->_globals->_eventMode = EVENTMODE_IGNORE;
				_vm->_events->refreshScreenAndEvents();
			} else {
				_vm->_globals->_eventMode = EVENTMODE_ALT;
				_vm->_events->refreshScreenAndEvents();
				_vm->_globals->_eventMode = EVENTMODE_IGNORE;
			}
		}
		curChar = *srcP++;
	}
}

} // End of namespace Hopkins
