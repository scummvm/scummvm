/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/util.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/cdrom.h"

namespace Gob {

Draw_v2::Draw_v2(GobEngine *vm) : Draw_v1(vm) {
}

void Draw_v2::printText(void) {
	int i;
	char *dataPtr;
	char *ptr;
	char *ptr2;
	char mask[80];
	char str[80];
	char buf[50];
	char cmd;
	int16 savedFlags;
	int16 destX;
	int16 destY;
	int16 spriteRight;
	int16 spriteBottom;
	int16 val;
	int16 index;
	int16 rectLeft;
	int16 rectTop;
	int16 rectRight;
	int16 rectBottom;
	int16 fontIndex;
	int16 strPos; // si
	int16 frontColor;
	int16 colId;
	int16 strPos2;
	int16 offX;
	int16 offY;
	int16 extraCmd;
	int16 strPosBak;
	int16 maskChar;
	int16 width;

	index = _vm->_inter->load16();

	_vm->_cdrom->playMultMusic();

	if (_vm->_game->_totTextData == 0)
		return;

	dataPtr = (char *)_vm->_game->_totTextData + _vm->_game->_totTextData->items[index].offset;
	ptr = dataPtr;

	if ((_renderFlags & 0x400) && (ptr[1] & 0x80))
		return;

	if (_renderFlags & RENDERFLAG_CAPTUREPUSH) {
		_destSpriteX = READ_LE_UINT16(ptr) & 0x7FFF;
		_destSpriteY = READ_LE_UINT16(ptr + 2);
		_spriteRight = READ_LE_UINT16(ptr + 4) - _destSpriteX + 1;
		_spriteBottom = READ_LE_UINT16(ptr + 6) - _destSpriteY + 1;
		_vm->_game->capturePush(_destSpriteX, _destSpriteY,
						 _spriteRight, _spriteBottom);
		(*_vm->_scenery->_pCaptureCounter)++;
	}
	
	_destSpriteX = READ_LE_UINT16(ptr) & 0x7FFF;
	destX = _destSpriteX;

	_destSpriteY = READ_LE_UINT16(ptr + 2);
	destY = _destSpriteY;

	_spriteRight = READ_LE_UINT16(ptr + 4);
	spriteRight = _spriteRight;

	_spriteBottom = READ_LE_UINT16(ptr + 6);
	spriteBottom = _spriteBottom;

	_destSurface = 21;

	ptr += 8;

	_backColor = *ptr++;
	_transparency = 1;

	spriteOperation(DRAW_CLEARRECT);

	_backColor = 0;

	savedFlags = _renderFlags;
	_renderFlags &= ~RENDERFLAG_NOINVALIDATE;

	for (; (_destSpriteX = READ_LE_UINT16(ptr)) != -1; ptr++) {
		_destSpriteX += destX;
		_destSpriteY = READ_LE_UINT16(ptr + 2) + destY;
		_spriteRight = READ_LE_UINT16(ptr + 4) + destX;
		_spriteBottom = READ_LE_UINT16(ptr + 6) + destY;
		ptr += 8;

		cmd = (*ptr & 0xf0) >> 4;
		if (cmd == 0) {
			_frontColor = *ptr & 0xf;
			spriteOperation(DRAW_DRAWLINE);
		} else if (cmd == 1) {
			_frontColor = *ptr & 0xf;
			spriteOperation(DRAW_DRAWBAR);
		} else if (cmd == 2) {
			_backColor = *ptr & 0xf;
			spriteOperation(DRAW_FILLRECTABS);
		}
	}
	ptr += 2;

	for (ptr2 = ptr; *ptr2 != 1; ptr2++) {
		if ((_vm->_game->_totFileData[0x29] < 0x32) && (*ptr2 > 3) && (*ptr2 < 32))
			*ptr2 = 32;

		switch (*ptr2) {
		case 1:
			break;

		case 2:
		case 5:
			ptr2 += 5;
			break;

		case 3:
		case 4:
			ptr2 += 2;
			break;

		case 6:
			ptr2++;
			switch (*ptr & 0xC0) {
			case 0x40:
				ptr2 += 9;
				break;
			case 0x80:
				ptr2 += 3;
				break;
			case 0xC0:
				ptr2 += 11;
				break;
			default:
				ptr2++;
				break;
			}
			break;

		case 10:
			ptr2 += (ptr2[1] * 2) + 2;
			break;

		default:
			ptr2++;
			break;
		}
	}

	ptr2++;

	fontIndex = 0;
	strPos = 0;
	extraCmd = 0;
	frontColor = 0;
	colId = 0;
	offX = 0;
	offY = 0;
	strPos2 = -1;
	memset(mask, 0, 80);
	memset(str, ' ', 80);
	maskChar = 0;
	_backColor = 0;
	_transparency = 1;
	
	while (true) {
		if ((*ptr >= 1) && ((*ptr <= 7) || (*ptr == 10)) && (strPos != 0)) {
			str[MAX(strPos, strPos2)] = 0;
			strPosBak = strPos;
			width = strlen(str) * _fonts[fontIndex]->itemWidth;
			adjustCoords(1, &width, 0);
			if (extraCmd & 0x0F) {
				rectLeft = offX - 2;
				rectTop = offY - 2;
				rectRight = offX + width + 1;
				rectBottom = _fonts[fontIndex]->itemHeight;
				adjustCoords(1, &rectBottom, 0);
				rectBottom += offY + 1;
				adjustCoords(0, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);
				if (colId != -1)
					_vm->_game->addNewCollision(colId & 0x0D000, rectLeft, rectTop,
							rectRight, rectBottom, 2, 0, 0, 0);
				if (_word_2E8E2 != 2)
					printTextCentered(extraCmd & 0x0F, rectLeft + 4, rectTop + 4,
							rectRight - 4, rectBottom - 4, str, fontIndex, frontColor);
				else
					printTextCentered(extraCmd & 0x0F, rectLeft + 2, rectTop + 2,
							rectRight - 2, rectBottom - 2, str, fontIndex, frontColor);
			} else {
				_destSpriteX = offX;
				_destSpriteY = offY;
				_fontIndex = fontIndex;
				_frontColor = frontColor;
				_textToPrint = str;
				if (_word_2E8E2 != 2) {
					if ((_destSpriteX >= destX) && (_destSpriteY >= destY)) {
						if (((_fonts[_fontIndex]->itemHeight / 2) + _destSpriteY - 1) <= spriteBottom) {
							while (((_destSpriteX + width - 1) > spriteRight) && (width > 0)) {
								width -= _fonts[_fontIndex]->itemWidth / 2;
								str[strlen(str) - 1] = '\0';
							}
							spriteOperation(DRAW_PRINTTEXT);
						}
					}
				} else
					spriteOperation(DRAW_PRINTTEXT);
				width = strlen(str);
				for (strPos = 0; strPos < width; strPos++) {
					if (mask[strPos] == '\0') continue;
					rectLeft = _fonts[fontIndex]->itemWidth;
					rectTop = _fonts[fontIndex]->itemHeight;
					adjustCoords(1, &rectLeft, &rectTop);
					_destSpriteX = strPos * rectLeft + offX;
					_spriteRight = _destSpriteX + rectLeft - 1;
					_spriteBottom = offY + rectTop;
					_destSpriteY = _spriteBottom;
					spriteOperation(DRAW_DRAWLINE);
				}
			}
			rectLeft = _fonts[_fontIndex]->itemWidth;
			adjustCoords(1, &rectLeft, 0);
			offX += strPosBak * rectLeft;
			strPos = 0;
			strPos2 = -1;
			memset(mask, 0, 80);
			memset(str, ' ', 80);
		}

		if (*ptr == 1)
			break;

		cmd = *ptr;
		switch ((uint8) cmd) {
		case 2:
		case 5:
			ptr++;
			offX = destX + (int16)READ_LE_UINT16(ptr);
			offY = destY + (int16)READ_LE_UINT16(ptr + 2);
			ptr += 4;
			break;

		case 3:
			ptr++;
			fontIndex = ((*ptr & 0xF0) >> 4) & 7;
			frontColor = *ptr & 0x0F;
			ptr++;
			break;

		case 4:
			ptr++;
			frontColor = *ptr++;
			break;

		case 6:
			ptr++;
			extraCmd = *ptr++;
			colId = -1;
			if (extraCmd & 0x80) {
				colId = (int16)READ_LE_UINT16(ptr);
				ptr += 2;
			}
			if (extraCmd & 0x40) {
				rectLeft = destX + (int16)READ_LE_UINT16(ptr);
				rectRight = destX + (int16)READ_LE_UINT16(ptr + 2);
				rectTop = destY + (int16)READ_LE_UINT16(ptr + 4);
				rectBottom = destY + (int16)READ_LE_UINT16(ptr + 6);
				adjustCoords(2, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);
				_vm->_game->addNewCollision(colId & 0x0D000, rectLeft, rectTop,
						rectRight, rectBottom, 2, 0, 0, 0);
				ptr += 8;
			}
			break;

		case 7:
			ptr++;
			extraCmd = 0;
			break;

		case 8:
			ptr++;
			maskChar = 1;
			break;

		case 9:
			ptr++;
			maskChar = 0;
			break;

		case 10:
			// loc_12C93
			str[0] = (char)255;
			WRITE_LE_UINT16((uint16*)(str+1), ptr - (char *)_vm->_game->_totTextData);
			str[3] = 0;
			ptr++;
			i = *ptr++;
			for (i = *ptr++; i > 0; i--) {
				mask[strPos++] = maskChar;
				ptr += 2;
			}
			break;

		default:
			str[strPos] = cmd;
		case 32:
			mask[strPos++] = maskChar;
			ptr++;
			break;

		case 186:
			cmd = ptr2[17] & 0x7f;
			if (cmd == 0) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
			} else if (cmd == 1) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;
				strcpy(buf, _vm->_global->_inter_variables + val);
			} else {
				val = READ_LE_UINT16(ptr2 + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
				if (buf[0] == '-') {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						_vm->_util->insertStr("0", buf, 1);
					}
				} else {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						_vm->_util->insertStr("0", buf, 0);
					}
				}
				if (_vm->_global->_language == 2)
					_vm->_util->insertStr(".", buf, strlen(buf) + 1 - ptr2[17]);
				else
					_vm->_util->insertStr(",", buf, strlen(buf) + 1 - ptr2[17]);
			}
			memcpy(str + strPos, buf, strlen(buf));
			memset(mask, maskChar, strlen(buf));
			if (ptr2[17] & 0x80) {
				strPos2 = strPos + strlen(buf);
				strPos++;
				ptr2 += 23;
				ptr++;
			} else {
				strPos += strlen(buf);
				if (ptr[1] != ' ') {
					if ((ptr[1] == 2) && (((int16)READ_LE_UINT16(ptr + 4)) == _destSpriteY)) {
						ptr += 5;
						str[strPos] = ' ';
						mask[strPos++] = maskChar;
					}
				} else {
					str[strPos] = ' ';
					mask[strPos++] = maskChar;
					while (ptr[1] == ' ')
						ptr++;
					if ((ptr[1] == 2) && (((int16)READ_LE_UINT16(ptr + 4)) == _destSpriteY))
						ptr += 5;
				}
				ptr2 += 23;
				ptr++;
			}
			break;
		}
	}

	_renderFlags = savedFlags;
	if (!(_renderFlags & 4))
		return;

	_vm->_game->checkCollisions(0, 0, 0, 0);

	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}

	return;
}

void Draw_v2::spriteOperation(int16 operation) {
	uint16 id;
	char *dataBuf;
	Game::TotResItem *itemPtr;
	int32 offset;
	int16 len;
	int16 i;
	int16 x;
	int16 y;
	Video::SurfaceDesc *sourceSurf;
	Video::SurfaceDesc *destSurf;
	bool deltaveto;
	int16 left;
	int16 ratio;
	int16 spriteLeft;
	int16 spriteTop;
	int16 spriteRight;
	int16 spriteBottom;
	int16 destSpriteX;
	int16 destSpriteY;
	int16 destSurface;
	int16 sourceSurface;
// .---
	int8 word_2F2D2 = -1;
// '---

	if (operation & 0x10) {
		deltaveto = true;
		operation &= 0x0F;
	} else
		deltaveto = false;

	if (_sourceSurface >= 100)
		_sourceSurface -= 80;

	if (_destSurface >= 100)
		_destSurface -= 80;

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaveto) {
		if ((_sourceSurface == 21) && (operation != DRAW_LOADSPRITE)) {
			_spriteLeft += _backDeltaX;
			_spriteTop += _backDeltaY;
		}

		if (_destSurface == 21) {
			_destSpriteX += _backDeltaX;
			_destSpriteY += _backDeltaY;
			if (operation == DRAW_DRAWLINE ||
			    (operation >= DRAW_DRAWBAR
				&& operation <= DRAW_FILLRECTABS)) {
				_spriteRight += _backDeltaX;
				_spriteBottom += _backDeltaY;
			}
		}
	}

	spriteLeft = _spriteLeft;
	spriteTop = _spriteTop;
	spriteRight = _spriteRight;
	spriteBottom = _spriteLeft;
	destSpriteX = _destSpriteX;
	destSpriteY = _destSpriteY;
	destSurface = _destSurface;
	sourceSurface = _sourceSurface;

//	warning("GOB2 Stub! _off_2E51B");
	if (_vm->_game->_off_2E51B != 0) {
		if ((_frontSurface->height <= _destSpriteY) &&
				((_destSurface == 20) || (_destSurface == 21))) {
			_destSpriteY -= _frontSurface->height;
			if (operation == DRAW_DRAWLINE ||
			    (operation >= DRAW_DRAWBAR
				&& operation <= DRAW_FILLRECTABS)) {
				_spriteBottom -= _frontSurface->height;
			}
			if (_destSurface == 21)
				invalidateRect(0, _frontSurface->height, 319, _frontSurface->height+_vm->_game->_off_2E51B->height-1);
			destSurface += 4;
		}
		if ((_frontSurface->height <= _spriteTop) && (operation == DRAW_BLITSURF)
				&& ((_destSurface == 20) || (_destSurface == 21))) {
			_spriteTop -= _frontSurface->height;
			_sourceSurface += 4;
		}
	}

	adjustCoords(0, &_destSpriteX, &_destSpriteY);
	if ((operation != DRAW_LOADSPRITE) && (_word_2E8E2 != 2)) {
		adjustCoords(0, &_spriteRight, &_spriteBottom);
		adjustCoords(0, &_spriteLeft, &_spriteTop);
		if (operation == DRAW_DRAWLETTER)
			operation = DRAW_BLITSURF;
		if ((operation == DRAW_DRAWLINE) &&
				((_spriteRight == _destSpriteX) || (_spriteBottom == _destSpriteY))) {
			operation = DRAW_FILLRECTABS;
			_backColor = _frontColor;
		}
		if (operation == DRAW_DRAWLINE) {
			if (_spriteBottom < _destSpriteY) {
				SWAP(_spriteBottom, _destSpriteY);
				SWAP(_spriteRight, _destSpriteX);
			}
		} else if ((operation == DRAW_LOADSPRITE) || (operation > DRAW_PRINTTEXT)) {
			if (_spriteBottom < _destSpriteY)
				SWAP(_spriteBottom, _destSpriteY);
			if (_spriteRight < _destSpriteX)
				SWAP(_spriteRight, _destSpriteX);
			_spriteRight++;
			_spriteBottom++;
		}
	}

	sourceSurf = _spritesArray[_sourceSurface];
	destSurf = _spritesArray[_destSurface];

	switch (operation) {
	case DRAW_BLITSURF:
	case DRAW_DRAWLETTER:
		if ((sourceSurf == 0) || (destSurf == 0))
			break;

		if ((sourceSurf->vidMode & 0x80) && (destSurf->vidMode & 0x80))
			_vm->_video->drawSprite(_spritesArray[_sourceSurface],
					_spritesArray[_destSurface],
					_spriteLeft, _spriteTop,
					_spriteLeft + _spriteRight - 1,
					_spriteTop + _spriteBottom - 1,
					_destSpriteX, _destSpriteY, _transparency);
		else if (!(sourceSurf->vidMode & 0x80) && (destSurf->vidMode & 0x80))
			drawSprite(_sourceSurface, _spritesArray[_destSurface],
					_spriteLeft, spriteTop,
					_spriteLeft + _spriteRight - 1,
					_spriteTop + _spriteBottom - 1,
					_destSpriteX, _destSpriteY, _transparency);
		else if ((sourceSurf->vidMode & 0x80) && !(destSurf->vidMode & 0x80))
			drawSprite(_spritesArray[_sourceSurface], _destSurface,
					_spriteLeft, spriteTop,
					_spriteLeft + _spriteRight - 1,
					_spriteTop + _spriteBottom - 1,
					_destSpriteX, _destSpriteY, _transparency);
		else
			drawSprite(_sourceSurface, _destSurface,
					_spriteLeft, _spriteTop,
					_spriteLeft + _spriteRight - 1,
					_spriteTop + _spriteBottom - 1,
					_destSpriteX, _destSpriteY, _transparency);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX + _spriteRight - 1,
			    _destSpriteY + _spriteBottom - 1);
		}
		break;

	case DRAW_PUTPIXEL:
		_vm->_video->putPixel(_destSpriteX, _destSpriteY,
		    _frontColor, _spritesArray[_destSurface]);
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX, _destSpriteY);
		}
		break;

	case DRAW_FILLRECT:
		fillRect(_destSurface, destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1,
				_destSpriteY + _spriteBottom - 1, _backColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX + _spriteRight - 1,
			    _destSpriteY + _spriteBottom - 1);
		}
		break;

	case DRAW_DRAWLINE:
		_vm->_video->drawLine(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_INVALIDATE:
		_vm->_video->drawCircle(_spritesArray[_destSurface], _destSpriteX, _destSpriteY, _spriteRight, _frontColor);
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX - _spriteRight, _destSpriteY - _spriteBottom,	// !!
			    _destSpriteX + _spriteRight,
			    _destSpriteY + _spriteBottom);
		}
		break;

	case DRAW_LOADSPRITE:
		id = _spriteLeft;
		if (id >= 30000) {
			dataBuf =
			    _vm->_game->loadExtData(id, &_spriteRight,
			    &_spriteBottom);
			_vm->_video->drawPackedSprite((byte *)dataBuf, _spriteRight,
			    _spriteBottom, _destSpriteX,
			    _destSpriteY, _transparency,
			    _spritesArray[_destSurface]);
			if (_destSurface == 21) {
				invalidateRect(_destSpriteX,
				    _destSpriteY,
				    _destSpriteX + _spriteRight - 1,
				    _destSpriteY + _spriteBottom - 1);
			}
			delete[] dataBuf;
			break;
		}
		// Load from .TOT resources
		itemPtr = &_vm->_game->_totResourceTable->items[id];
		offset = itemPtr->offset;
		if (offset >= 0) {
			dataBuf =
			    ((char *)_vm->_game->_totResourceTable) +
			    szGame_TotResTable + szGame_TotResItem *
			    _vm->_game->_totResourceTable->itemsCount + offset;
		} else {
			dataBuf =
			    _vm->_game->_imFileData +
			    (int32)READ_LE_UINT32(&((int32 *)_vm->_game->_imFileData)[-offset - 1]);
		}

		_spriteRight = itemPtr->width;
		_spriteBottom = itemPtr->height;
		_vm->_video->drawPackedSprite((byte *)dataBuf,
		    _spriteRight, _spriteBottom,
		    _destSpriteX, _destSpriteY,
		    _transparency, _spritesArray[_destSurface]);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX + _spriteRight - 1,
			    _destSpriteY + _spriteBottom - 1);
		}
		break;

	case DRAW_PRINTTEXT:
		len = strlen(_textToPrint);
		left = _destSpriteX;
		if ((_fontIndex >= 4) || (_fontToSprite[_fontIndex].sprite == -1)) {
			if (_fonts[_fontIndex]->extraData == 0) {
				if (((signed) _textToPrint[0]) == -1) {
					dataBuf = (char*)_vm->_game->_totTextData + _textToPrint[1] + 1;
					len = *dataBuf++;
					for (i = 0; i < len; i++) {
						_vm->_video->drawLetter(READ_LE_UINT16(dataBuf), _destSpriteX,
								_destSpriteY, _fonts[_fontIndex], _transparency, _frontColor,
								_backColor, _spritesArray[_destSurface]);
						dataBuf += 2;
					}
				} else {
					drawString(_textToPrint, _destSpriteX, _destSpriteY, _frontColor,
							_backColor, _transparency, _spritesArray[_destSurface],
							_fonts[_fontIndex]);
					_destSpriteX += len * _fonts[_fontIndex]->itemWidth;
				}
			} else {
				if (word_2F2D2 >= 0) {
					for (i = 0; i < len; i++) {
						_vm->_video->drawLetter(_textToPrint[i], _destSpriteX,
								_destSpriteY, _fonts[_fontIndex], _transparency,
								_frontColor, _backColor, _spritesArray[_destSurface]);
						_destSpriteX +=
							*(((char*)_fonts[_fontIndex]->extraData) + (_textToPrint[i] - _fonts[_fontIndex]->startItem));
					}
				} else { // loc_DBE9
					warning("Untested, does that work?");
					// Does something different for each character depending on whether it's a space
					// That *should* be it...
					for (i = 0; i < len; i++) {
						if (_textToPrint[i] == ' ')
							_destSpriteX += _fonts[_fontIndex]->itemWidth;
						else {
							_vm->_video->drawLetter(_textToPrint[i],
									_destSpriteX, _destSpriteY,
									_fonts[_fontIndex],
									_transparency,
									_frontColor, _backColor,
									_spritesArray[_destSurface]);
							_destSpriteX +=
								*(((char*)_fonts[_fontIndex]->extraData) + (_textToPrint[i] - _fonts[_fontIndex]->startItem));
						}
					}
				}
			}
		} else {
			for (i = 0; i < len; i++) {
				ratio = _spritesArray[_fontToSprite[_fontIndex].sprite]->width
					/ _fontToSprite[_fontIndex].width;
				y = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) / ratio)
					* _fontToSprite[_fontIndex].height;
				x = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) % ratio)
					* _fontToSprite[_fontIndex].width;
				_vm->_video->drawSprite(_spritesArray[_fontToSprite[_fontIndex].sprite],
						_spritesArray[_destSurface], x, y,
						x + _fontToSprite[_fontIndex].width - 1,
						y + _fontToSprite[_fontIndex].height - 1,
						_destSpriteX, _destSpriteY, _transparency);
				_destSpriteX += _fontToSprite[_fontIndex].width;
			}
		}

		if (_destSurface == 21) {
			invalidateRect(left, _destSpriteY,
					_destSpriteX - 1,
					_destSpriteY + _fonts[_fontIndex]->itemHeight - 1);
		}
		break;

	case DRAW_DRAWBAR:
		if (_word_2E8E2 != 2) {
			_vm->_video->fillRect(_spritesArray[_destSurface],
					_destSpriteX, _spriteBottom - 1,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->fillRect(_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_destSpriteX + 1, _spriteBottom, _frontColor);

			_vm->_video->fillRect(_spritesArray[_destSurface],
					_spriteRight - 1, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->fillRect(_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY + 1, _frontColor);
		} else {
			_vm->_video->drawLine(_spritesArray[_destSurface],
					_destSpriteX, _spriteBottom,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->drawLine(_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_destSpriteX, _spriteBottom, _frontColor);

			_vm->_video->drawLine(_spritesArray[_destSurface],
					_spriteRight, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->drawLine(_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY, _frontColor);
		}
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_CLEARRECT:
		if ((_backColor != 16) && (_backColor != 144)) {
			_vm->_video->fillRect(_spritesArray[_destSurface],
			    _destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    _backColor);
		}
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_FILLRECTABS:
		_vm->_video->fillRect(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _backColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;
	}

	_spriteLeft = spriteLeft;
	_spriteTop = spriteTop;
	_spriteRight = spriteRight;
	_spriteLeft = spriteBottom;
	_destSpriteX = destSpriteX;
	_destSpriteY = destSpriteY;
	_destSurface = destSurface;
	_sourceSurface = sourceSurface;

	if (operation == DRAW_PRINTTEXT) {
		len = _fonts[_fontIndex]->itemWidth;
		adjustCoords(1, &len, 0);
		_destSpriteX += len * strlen(_textToPrint);
	}

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaveto) {
		if (_sourceSurface == 21) {
			_spriteLeft -= _backDeltaX;
			_spriteTop -= _backDeltaY;
		}

		if (_destSurface == 21) {
			_destSpriteX -= _backDeltaX;
			_destSpriteY -= _backDeltaY;
		}
	}
}

} // End of namespace Gob
