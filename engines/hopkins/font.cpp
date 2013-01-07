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
 */

#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/font.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

FontManager::FontManager() {
	clearAll();
}

void FontManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void FontManager::clearAll() {
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

	_tempText = g_PTRNUL;
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

	txt._textBlock = _vm->_globals.freeMemory(txt._textBlock);
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
	txt._textBlock = _vm->_globals.freeMemory(txt._textBlock);
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
void FontManager::initTextBuffers(int idx, int messageId, const Common::String &filename, int xp, int yp, int textType, int a9, int color) {
	if ((idx - 5) > MAX_TEXT)
		error("Attempted to display text > MAX_TEXT.");

	TxtItem &txt = _text[idx - 5];
	txt._textOnFl = false;
	txt._filename = filename;
	txt._pos.x = xp;
	txt._pos.y = yp;
	txt._messageId = messageId;
	txt._textType = textType;
	txt._field3FE = a9;
	txt._color = color;
}

// Box
void FontManager::box(int idx, int messageId, const Common::String &filename, int xp, int yp) {
	byte *v9;
	byte *ptre;
	Common::String s;
	Common::String file;
	Common::File f;

	int v73 = xp;
	if (idx < 0)
		error("Bad number for text");
	_vm->_globals.police_l = 11;

	_vm->_globals._boxWidth = 11 * _text[idx]._field3FE;
	if (_text[idx]._textLoadedFl) {
		int textType = _text[idx]._textType;
		if (textType != 6 && textType != 1 && textType != 3 && textType != 5) {
			int yCurrent = yp + 5;
			for (int lineNum = 0; lineNum < _text[idx]._lineCount; ++lineNum) {
				displayText(xp + 5, yCurrent, _text[idx]._lines[lineNum], _text[idx]._color);
				yCurrent += _vm->_globals.police_h + 1;
			}
		} else {
			int height = _text[idx]._height;
			int width = _text[idx]._width;
			_vm->_graphicsManager.Restore_Mem(
				_vm->_graphicsManager._vesaBuffer,
				_text[idx]._textBlock,
			    xp,
			    yp,
			    _text[idx]._width,
			    _text[idx]._height);
			_vm->_graphicsManager.addVesaSegment(xp, yp, xp + width, yp + height);
		}
	} else {
		int lineCount = 0;
		for (int v62 = 0; v62 <= 19; v62++)
			_textSortArray[v62] = 0;

		_text[idx]._textLoadedFl = true;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, filename);

		file = _vm->_globals._curFilename;
		if (strncmp(file.c_str(), _oldName.c_str(), strlen(file.c_str())) != 0) {
			// Starting to access a new file, so read in the index file for the file
			_oldName = file;
			_indexName = Common::String(file.c_str(), file.size() - 3);
			_indexName += "IND";

			if (!f.open(_indexName))
				error("Error opening file - %s", _indexName.c_str());
			int filesize = f.size();
			for (int i = 0; i < (filesize / 4); ++i)
				_index[i] = f.readUint32LE();
			f.close();
		}
		int v11, v69;
		if (filename[0] != 'Z' || filename[1] != 'O') {
			if (!f.open(file))
				error("Error opening file - %s", _indexName.c_str());

			v69 = 2048;
			f.seek(_index[messageId]);

			_tempText = _vm->_globals.allocMemory(2058);
			if (_tempText == g_PTRNUL)
				error("Error allocating text");

			Common::fill(&_tempText[0], &_tempText[2058], 0);
			f.read(_tempText, 2048);
			f.close();
			_vm->_globals.texte_long = 2048;
		} else {
			v69 = 100;
			_vm->_globals.texte_long = 100;
			v9 = _vm->_globals.allocMemory(110);
			Common::fill(&v9[0], &v9[110], 0);

			_tempText = v9;
			const byte *v10 = _vm->_globals.BUF_ZONE + _index[messageId];
			memcpy(v9, v10, 96);
			v11 = 0;
			WRITE_LE_UINT16((uint16 *)v9 + 48, (int16)READ_LE_UINT16(v10 + 96));
		}
		byte *v59 = _tempText;
		byte *v60;
		if (!v69)
			goto LABEL_43;
		for (int v63 = 0; v63 < v69; v63++) {
			byte v13 = *v59;
			if ((byte)(*v59 + 46) > 27) {
				if ((byte)(v13 + 80) > 27) {
					if ((byte)(v13 - 65) <= 25 || (byte)(v13 - 97) <= 25)
						v13 = 32;
				} else {
					v13 -= 79;
				}
			} else {
				v13 += 111;
			}
			*v59 = v13;
			v59++;
		};

		v60 = _tempText;
		if (v69) {
			int v64 = 0;
			for (;;) {
				byte v14 = v60[v64];
				if (v14 == '\r' || v14 == '\n') {
					v60[v64] = 0;
					if (!_text[idx]._field3FE)
						break;
				}
				++v64;
				if (v69 <= v64)
					goto LABEL_43;
			}
			_text[idx]._field3FE = v64;
			_vm->_globals._boxWidth = 0;

			for (int v15 = 0; v15 < v64 + 1; v15++) {
				byte v16 = v60[v15];
				if (v16 <= 31)
					v16 = ' ';
				_vm->_globals._boxWidth += _vm->_objectsManager.getWidth(_vm->_globals.police, v16 - 32);
			}

			_vm->_globals._boxWidth += 2;
			int v17 = _vm->_globals._boxWidth / 2;
			if (v17 < 0)
				v17 = -v17;
			_text[idx]._pos.x = 320 - v17;
			v73 = _vm->_eventsManager._startPos.x + 320 - v17;
			lineCount = 1;
			if (v64 + 1 > 0) {
				_text[idx]._lines[0] = Common::String((const char *)v60, v64);
			}
		} else {
LABEL_43:
			if (!_vm->_globals._boxWidth)
				_vm->_globals._boxWidth = 240;
			int v65 = 0;
			byte *v61 = _tempText;
			int lineSize;
			do {
				int v19 = 0;
				int ptrb = _vm->_globals._boxWidth - 4;
				for (;;) {
					lineSize = v19;
					do
						v11 = v61[v65 + v19++];
					while (v11 != ' ' && v11 != '%');
					if (v19 >= ptrb / _vm->_globals.police_l)
						break;
					if (v11 == '%') {
						if (v19 < ptrb / _vm->_globals.police_l)
							goto LABEL_55;
						break;
					}
				}
				if (v11 != '%')
					goto LABEL_57;
				v11 = ' ';
LABEL_55:
				if (v11 == '%')
					lineSize = v19;
LABEL_57:
				int v20 = lineCount;

				// WORKAROUND: Perhaps due to the usage of ScummVM strings here, recalculate what the
				// actual length of the line to be copied will be. Otherwise, you can see artifacts,
				// such as a single character beyond the end of string NULL.
				int actualSize = 0;
				while (actualSize < lineSize && v61[v65 + actualSize])
					++actualSize;

				_text[idx]._lines[v20] = Common::String((const char *)v61 + v65, actualSize);
				_textSortArray[lineCount++] = lineSize;

				v65 += lineSize;
			} while (v11 != '%');

			for (int i = 0; i <= 19; i++) {
				if (_textSortArray[i] <= 0) {
					_textSortArray[i] = 0;
				} else {
					int ptrc = 0;
					for (int v23 = 0; v23 < _textSortArray[i] - 1; v23++) {
						Common::String &line = _text[idx]._lines[i];
						byte v24 = (v23 >= (int)line.size()) ? '\0' : line.c_str()[v23];
						if (v24 <= 32)
							v24 = ' ';
						ptrc += _vm->_objectsManager.getWidth(_vm->_globals.police, (byte)v24 - 32);
					}
					_textSortArray[i] = ptrc;
				}
			}
			for (int i = 0; i <= 19; i++) {
				for (int v25 = i + 1; v25 != i; v25 = (v25 + 1) % 20) {
					if (_textSortArray[i] < _textSortArray[v25])
						_textSortArray[i] = 0;
				}
			};

			for (int i = 0; i <= 19; i++) {
				if (_textSortArray[i])
					_vm->_globals._boxWidth = _textSortArray[i];
			}

			if ((_text[idx]._textType < 2) || (_text[idx]._textType > 3)) {
				int i;
				for (i = xp - _vm->_eventsManager._startPos.x; _vm->_globals._boxWidth + i > 638 && i > -2 && _text[idx]._textType; i -= 2)
					;
				_text[idx]._pos.x = i;
				v73 = _vm->_eventsManager._startPos.x + i;
			} else {
				if (_vm->_globals.nbrligne == (SCREEN_WIDTH - 1)) {
					while (_vm->_globals._boxWidth + v73 > 638 && v73 > -2)
						v73 -= 2;
				}
				if (_vm->_globals.nbrligne == (SCREEN_WIDTH * 2)) {
					while (_vm->_globals._boxWidth + v73 > 1278 && v73 > -2)
						v73 -= 2;
				}
				_text[idx]._pos.x = v73;
			}
		}
		int posX = v73;
		int posY = yp;
		int saveWidth = _vm->_globals._boxWidth + 10;
		int saveHeight = (_vm->_globals.police_h + 1) * lineCount + 12;
		if (_text[idx]._textType == 6) {
			int v27 = saveWidth / 2;
			if (v27 < 0)
				v27 = -v27;
			_text[idx]._pos.x = 315 - v27;
			int v28 = _vm->_eventsManager._startPos.x + 315 - v27;
			v73 = _vm->_eventsManager._startPos.x + 315 - v27;
			_text[idx]._pos.y = 50;
			posY = 50;
			posX = v28;
		}
		int textType = _text[idx]._textType;
		if (textType == 1 || textType == 3 || textType == 5 || textType == 6) {
			int size = saveHeight * saveWidth;
			byte *ptrd = _vm->_globals.allocMemory(size);
			if (ptrd == g_PTRNUL)
				error("Cutting a block for text box (%d)", size);

			_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager._vesaBuffer, ptrd, posX, posY, saveWidth, saveHeight);
			_vm->_graphicsManager.Trans_bloc2(ptrd, _vm->_graphicsManager._colorTable, size);
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, ptrd, posX, posY, saveWidth, saveHeight);
			_vm->_globals.freeMemory(ptrd);

			_vm->_graphicsManager.drawHorizontalLine(_vm->_graphicsManager._vesaBuffer, posX, posY, saveWidth, (byte)-2);
			_vm->_graphicsManager.drawHorizontalLine(_vm->_graphicsManager._vesaBuffer, posX, saveHeight + posY, saveWidth, (byte)-2);
			_vm->_graphicsManager.drawVerticalLine(_vm->_graphicsManager._vesaBuffer, posX, posY, saveHeight, (byte)-2);
			_vm->_graphicsManager.drawVerticalLine(_vm->_graphicsManager._vesaBuffer, saveWidth + posX, posY, saveHeight, (byte)-2);
		}
		_text[idx]._lineCount = lineCount;
		int v71 = posY + 5;

		for (int lineNum = 0; lineNum < lineCount; ++lineNum) {
			displayText(v73 + 5, v71, _text[idx]._lines[lineNum], _text[idx]._color);
			v71 += _vm->_globals.police_h + 1;
		}

		int blockWidth = saveWidth + 1;
		int blockHeight = saveHeight + 1;

		_text[idx]._width = blockWidth;
		_text[idx]._height = blockHeight;
		textType = _text[idx]._textType;
		if (textType == 6 || textType == 1 || textType == 3 || textType == 5) {
			_text[idx]._textBlock = _vm->_globals.freeMemory(_text[idx]._textBlock);
			int blockSize = blockHeight * blockWidth;
			ptre = _vm->_globals.allocMemory(blockSize + 20);
			if (ptre == g_PTRNUL)
				error("Cutting a block for text box (%d)", blockSize);

			_text[idx]._textBlock = ptre;
			_text[idx]._width = blockWidth;
			_text[idx]._height = blockHeight;
			_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager._vesaBuffer, _text[idx]._textBlock, posX, posY, _text[idx]._width, blockHeight);
		}
		_tempText = _vm->_globals.freeMemory(_tempText);
	}
}

/**
 * Directly display text (using a VESA segment)
 */
void FontManager::displayTextVesa(int xp, int yp, const Common::String &message, int col) {
	char currChar;
	int charIndex;
	int currentX = xp;

	const char *srcP = message.c_str();
	for (;;) {
		currChar = *srcP++;
		if (!currChar)
			break;
		if (currChar >= 32) {
			charIndex = currChar - 32;
			_vm->_graphicsManager.displayFont(_vm->_graphicsManager._vesaBuffer, _vm->_globals.police,
				currentX, yp, currChar - 32, col);
			currentX += _vm->_objectsManager.getWidth(_vm->_globals.police, charIndex);
		}
	}

	_vm->_graphicsManager.addVesaSegment(xp, yp, currentX, yp + 12);
}

/**
 * Directly display text
 */
void FontManager::displayText(int xp, int yp, const Common::String &message, int col) {
	for (uint idx = 0; idx < message.size(); ++idx) {
		char currentChar = message[idx];

		if (currentChar > 31) {
			int characterIndex = currentChar - 32;
			_vm->_graphicsManager.displayFont(_vm->_graphicsManager._vesaBuffer, _vm->_globals.police, xp, yp, characterIndex, col);
			xp += _vm->_objectsManager.getWidth(_vm->_globals.police, characterIndex);
		}
	}
}

void FontManager::TEXT_COMPUT(int xp, int yp, const Common::String &msg, int col) {
	const char *srcP;
	int v5;
	int v6;
	byte v7;
	int fontCol;
	int v9;

	srcP = msg.c_str();
	v9 = xp;
	fontCol = col;
	do {
		v7 = *srcP++;
		if (v7 == '&') {
			fontCol = 2;
			v7 = *srcP++;
		}
		if (v7 == '$') {
			fontCol = 4;
			v7 = *srcP++;
		}
		if (!v7)
			break;
		if (v7 >= 32) {
			v5 = v7 - 32;
			_vm->_graphicsManager.displayFont(_vm->_graphicsManager._vesaBuffer, _vm->_globals.police, v9, yp, v7 - 32, fontCol);
			v9 += _vm->_objectsManager.getWidth(_vm->_globals.police, v5);
			v6 = _vm->_objectsManager.getWidth(_vm->_globals.police, v5);
			_vm->_graphicsManager.addVesaSegment(v9 - v6, yp, v9, yp + 12);
			if (_vm->_eventsManager._escKeyFl) {
				_vm->_globals.iRegul = 1;
				_vm->_eventsManager.VBL();
			} else {
				_vm->_globals.iRegul = 4;
				_vm->_eventsManager.VBL();
				_vm->_globals.iRegul = 1;
			}
		}
	} while (v7);
}

} // End of namespace Hopkins
