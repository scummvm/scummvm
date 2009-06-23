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

#include "common/endian.h"
#include "graphics/cursorman.h"

#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/video.h"

namespace Gob {

Draw_v2::Draw_v2(GobEngine *vm) : Draw_v1(vm) {
	_mayorWorkaroundStatus = 0;
}

void Draw_v2::initScreen() {
	_scrollOffsetX = 0;
	_scrollOffsetY = 0;

	initSpriteSurf(21, _vm->_video->_surfWidth, _vm->_video->_surfHeight, 0);
	_backSurface = _spritesArray[21];
	_vm->_video->clearSurf(*_backSurface);

	if (!_spritesArray[23]) {
		initSpriteSurf(23, 32, 16, 2);
		_cursorSpritesBack = _spritesArray[23];
		_cursorSprites = _cursorSpritesBack;
		_scummvmCursor =
			_vm->_video->initSurfDesc(_vm->_global->_videoMode, 16, 16, SCUMMVM_CURSOR);
	}

	_spritesArray[20] = _frontSurface;
	_spritesArray[21] = _backSurface;

	_vm->_video->dirtyRectsAll();
}

void Draw_v2::closeScreen() {
	//freeSprite(23);
	//_cursorSprites = 0;
	//_cursorSpritesBack = 0;
	//_scummvmCursor = 0;
	freeSprite(21);
}

void Draw_v2::blitCursor() {
	if (_cursorIndex == -1)
		return;

	_showCursor = (_showCursor & ~2) | ((_showCursor & 1) << 1);
}

void Draw_v2::animateCursor(int16 cursor) {
	Game::Collision *ptr;
	int16 cursorIndex = cursor;
	int16 newX = 0, newY = 0;
	uint16 hotspotX = 0, hotspotY = 0;

	_showCursor |= 1;

	// .-- _draw_animateCursorSUB1 ---
	if (cursorIndex == -1) {
		cursorIndex = 0;
		for (ptr = _vm->_game->_collisionAreas; ptr->left != 0xFFFF; ptr++) {
			if ((ptr->flags & 0xF00) || (ptr->id & 0x4000))
				continue;

			if (ptr->left > _vm->_global->_inter_mouseX)
				continue;

			if (ptr->right < _vm->_global->_inter_mouseX)
				continue;

			if (ptr->top > _vm->_global->_inter_mouseY)
				continue;

			if (ptr->bottom < _vm->_global->_inter_mouseY)
				continue;

			if ((ptr->flags & 0xF000) == 0) {
				if ((ptr->flags & 0xF) >= 3) {
					cursorIndex = 3;
					break;
				} else if (((ptr->flags & 0xF0) != 0x10) && (cursorIndex == 0))
					cursorIndex = 1;
			} else if (cursorIndex == 0)
				cursorIndex = (ptr->flags >> 12) & 0xF;
		}
		if (_cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}
	// '------

	if (_cursorAnimLow[cursorIndex] != -1) {
		// .-- _draw_animateCursorSUB2 ---
		if (cursorIndex == _cursorIndex) {
			if ((_cursorAnimDelays[_cursorIndex] != 0) &&
					((_cursorTimeKey + (_cursorAnimDelays[_cursorIndex] * 10)) <=
						_vm->_util->getTimeKey())) {
				_cursorAnim++;
				if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
						(_cursorAnimLow[_cursorIndex] > _cursorAnim))
					_cursorAnim = _cursorAnimLow[_cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			} else {
				if (_noInvalidated && (_vm->_global->_inter_mouseX == _cursorX) &&
						(_vm->_global->_inter_mouseY == _cursorY)) {
					_vm->_video->waitRetrace();
					return;
				}
			}
		} else {
			_cursorIndex = cursorIndex;
			if (_cursorAnimDelays[cursorIndex] != 0) {
				_cursorAnim = _cursorAnimLow[cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			}
		}

		if (_cursorAnimDelays[_cursorIndex] != 0) {
			if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
					(_cursorAnimLow[_cursorIndex] > _cursorAnim))
				_cursorAnim = _cursorAnimLow[_cursorIndex];

			cursorIndex = _cursorAnim;
		}
		// '------

		newX = _vm->_global->_inter_mouseX;
		newY = _vm->_global->_inter_mouseY;
		if (_cursorHotspotXVar != -1) {
			newX -= hotspotX = (uint16) VAR(_cursorIndex + _cursorHotspotXVar);
			newY -= hotspotY = (uint16) VAR(_cursorIndex + _cursorHotspotYVar);
		}

		_vm->_video->clearSurf(*_scummvmCursor);
		_vm->_video->drawSprite(*_cursorSprites, *_scummvmCursor,
				cursorIndex * _cursorWidth, 0,
				(cursorIndex + 1) * _cursorWidth - 1,
				_cursorHeight - 1, 0, 0, 0);
		CursorMan.replaceCursor(_scummvmCursor->getVidMem(),
				_cursorWidth, _cursorHeight, hotspotX, hotspotY, 0);

		if (_frontSurface != _backSurface) {
			if (!_noInvalidated) {
				int16 tmp = _cursorIndex;
				_cursorIndex = -1;
				blitInvalidated();
				_cursorIndex = tmp;
			} else {
				_showCursor = 3;
				_vm->_video->waitRetrace();
				if (MIN(newY, _cursorY) < 50)
					_vm->_util->delay(5);
			}
		}
	} else {
		blitCursor();
		_cursorX = newX;
		_cursorY = newY;
	}

	_showCursor &= ~1;
}

void Draw_v2::printTotText(int16 id) {
	byte *dataPtr;
	byte *ptr, *ptrEnd;
	byte cmd;
	int16 savedFlags;
	int16 destX, destY;
	int16 spriteRight, spriteBottom;
	int16 val;
	int16 rectLeft, rectTop, rectRight, rectBottom;
	int16 size;

	id &= 0xFFF;

	if (!_vm->_game->_totTextData || !_vm->_game->_totTextData->dataPtr ||
	    (id >= _vm->_game->_totTextData->itemsCount) ||
		  (_vm->_game->_totTextData->items[id].offset == 0xFFFF) ||
			(_vm->_game->_totTextData->items[id].size == 0))
		return;

	_vm->validateLanguage();

	// WORKAROUND: In the scripts of some Gobliins 2 versions, the dialog text IDs
	// for Fingus and the mayor are swapped.
	if ((_vm->getGameType() == kGameTypeGob2) && !_vm->isCD() &&
	    (!scumm_stricmp(_vm->_game->_curTotFile, "gob07.tot"))) {

		if (id == 24) {
			if (_mayorWorkaroundStatus == 1) {
				_mayorWorkaroundStatus = 0;
				id = 31;
			} else
				_mayorWorkaroundStatus = 2;
		} else if (id == 31) {
			if (_mayorWorkaroundStatus == 0) {
				_mayorWorkaroundStatus = 1;
				id = 24;
			} else
				_mayorWorkaroundStatus = 0;
		}

	}

	size = _vm->_game->_totTextData->items[id].size;
	dataPtr = _vm->_game->_totTextData->dataPtr +
		_vm->_game->_totTextData->items[id].offset;
	ptr = dataPtr;

	if ((_renderFlags & RENDERFLAG_SKIPOPTIONALTEXT) && (ptr[1] & 0x80))
		return;

	if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
		destX = (READ_LE_UINT16(ptr) & 0x7FFF) * 2;
		spriteRight = READ_LE_UINT16(ptr + 4) * 2 + 1;
	} else {
		destX = READ_LE_UINT16(ptr) & 0x7FFF;
		spriteRight = READ_LE_UINT16(ptr + 4);
	}

	if (_renderFlags & RENDERFLAG_FROMSPLIT) {
		int16 start;

		start = _vm->_video->_splitStart;

		destY = start;
		spriteBottom = READ_LE_UINT16(ptr + 6) - READ_LE_UINT16(ptr + 2);

		if (_renderFlags & RENDERFLAG_DOUBLECOORDS)
			spriteBottom *= 3;

		spriteBottom += start;

		if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
			spriteBottom += _backDeltaY;
			destY += _backDeltaY;
		}
	} else {
		destY = READ_LE_UINT16(ptr + 2);
		spriteBottom = READ_LE_UINT16(ptr + 6);

		if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
			destY *= 2;
			spriteBottom *= 2;
		}
	}

	ptr += 8;

	if (_renderFlags & RENDERFLAG_CAPTUREPUSH) {
		_vm->_game->capturePush(destX, destY,
				spriteRight - destX + 1, spriteBottom - destY + 1);
		(*_vm->_scenery->_pCaptureCounter)++;
	}

	_destSpriteX = destX;
	_destSpriteY = destY;
	_spriteRight = spriteRight;
	_spriteBottom = spriteBottom;
	_destSurface = 21;

	_backColor = *ptr++;
	_transparency = 1;

	spriteOperation(DRAW_CLEARRECT);

	_backColor = 0;
	savedFlags = _renderFlags;
	_renderFlags &= ~RENDERFLAG_NOINVALIDATE;

	while ((_destSpriteX = READ_LE_UINT16(ptr)) != -1) {
		_destSpriteX += destX;
		_destSpriteY = READ_LE_UINT16(ptr + 2) + destY;
		_spriteRight = READ_LE_UINT16(ptr + 4) + destX;
		_spriteBottom = READ_LE_UINT16(ptr + 6) + destY;
		ptr += 8;

		cmd = *ptr++;
		switch ((cmd & 0xF0) >> 4) {
		case 0:
			_frontColor = cmd & 0xF;
			spriteOperation(DRAW_DRAWLINE);
			break;
		case 1:
			_frontColor = cmd & 0xF;
			spriteOperation(DRAW_DRAWBAR);
			break;
		case 2:
			_backColor = cmd & 0xF;
			spriteOperation(DRAW_FILLRECTABS);
			break;
		}
	}
	ptr += 2;

	ptrEnd = ptr;
	while (((ptrEnd - dataPtr) < size) && (*ptrEnd != 1)) {
		// Converting to unknown commands/characters to spaces
		if ((_vm->_game->_script->getVersionMinor() < 2) && (*ptrEnd > 3) && (*ptrEnd < 32))
			*ptrEnd = 32;

		switch (*ptrEnd) {
		case 1:
			break;

		case 2:
		case 5:
			ptrEnd += 5;
			break;

		case 3:
		case 4:
			ptrEnd += 2;
			break;

		case 6:
			ptrEnd++;
			switch (*ptrEnd & 0xC0) {
			case 0x40:
				ptrEnd += 9;
				break;
			case 0x80:
				ptrEnd += 3;
				break;
			case 0xC0:
				ptrEnd += 11;
				break;
			default:
				ptrEnd++;
				break;
			}
			break;

		case 10:
			ptrEnd += (ptrEnd[1] * 2) + 2;
			break;

		default:
			ptrEnd++;
			break;
		}
	}
	ptrEnd++;

	int16 fontIndex = 0, frontColor = 0;
	int16 strPos = 0, strPos2 = -1, strPosBak;
	int16 offX = 0, offY = 0;
	int16 colId = 0;
	int16 colCmd = 0;
	int16 width;
	int16 maskChar = 0;
	char mask[80], str[80], buf[50];

	memset(mask, 0, 80);
	memset(str, ' ', 80);
	_backColor = 0;
	_transparency = 1;

	while (true) {
		if ((((*ptr >= 1) && (*ptr <= 7)) || (*ptr == 10)) && (strPos != 0)) {
			str[MAX(strPos, strPos2)] = 0;
			strPosBak = strPos;
			width = strlen(str) * _fonts[fontIndex]->itemWidth;
			adjustCoords(1, &width, 0);

			if (colCmd & 0x0F) {
				rectLeft = offX - 2;
				rectTop = offY - 2;
				rectRight = offX + width + 1;
				rectBottom = _fonts[fontIndex]->itemHeight;
				adjustCoords(1, &rectBottom, 0);
				rectBottom += offY + 1;
				adjustCoords(0, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);

				if (colId != -1)
					_vm->_game->addNewCollision(colId + 0xD000, rectLeft, rectTop,
							rectRight, rectBottom, 2, 0, 0, 0);

				if (_needAdjust != 2)
					printTextCentered(colCmd & 0x0F, rectLeft + 4, rectTop + 4,
							rectRight - 4, rectBottom - 4, str, fontIndex, frontColor);
				else
					printTextCentered(colCmd & 0x0F, rectLeft + 2, rectTop + 2,
							rectRight - 2, rectBottom - 2, str, fontIndex, frontColor);

			} else {
				_destSpriteX = offX;
				_destSpriteY = offY;
				_fontIndex = fontIndex;
				_frontColor = frontColor;
				_textToPrint = str;

				if (_needAdjust != 2) {
					if ((_destSpriteX >= destX) && (_destSpriteY >= destY) &&
					    (((_fonts[_fontIndex]->itemHeight / 2) + _destSpriteY - 1) <= spriteBottom)) {
						while (((_destSpriteX + width - 1) > spriteRight) && (width > 0)) {
							width -= _fonts[_fontIndex]->itemWidth / 2;
							str[strlen(str) - 1] = '\0';
						}
						spriteOperation(DRAW_PRINTTEXT);
					}
				} else
					spriteOperation(DRAW_PRINTTEXT);

				width = strlen(str);
				for (strPos = 0; strPos < width; strPos++) {
					if (mask[strPos] == '\0')
						continue;

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
			colCmd = *ptr++;
			colId = -1;
			if (colCmd & 0x80) {
				colId = (int16)READ_LE_UINT16(ptr);
				ptr += 2;
			}
			if (colCmd & 0x40) {
				rectLeft = destX + (int16)READ_LE_UINT16(ptr);
				rectRight = destX + (int16)READ_LE_UINT16(ptr + 2);
				rectTop = destY + (int16)READ_LE_UINT16(ptr + 4);
				rectBottom = destY + (int16)READ_LE_UINT16(ptr + 6);
				adjustCoords(2, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);
				_vm->_game->addNewCollision(colId + 0x0D000, rectLeft, rectTop,
						rectRight, rectBottom, 2, 0, 0, 0);
				ptr += 8;
			}
			break;

		case 7:
			ptr++;
			colCmd = 0;
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
			str[0] = (char) 255;
			WRITE_LE_UINT16((uint16 *) (str + 1),
					ptr - _vm->_game->_totTextData->dataPtr);
			str[3] = 0;
			ptr++;
			for (int i = *ptr++; i > 0; i--) {
				mask[strPos++] = maskChar;
				ptr += 2;
			}
			break;

		default:
			str[strPos] = (char) cmd;
		case 32:
			mask[strPos++] = maskChar;
			ptr++;
			break;

		case 186:
			cmd = ptrEnd[17] & 0x7F;
			if (cmd == 0) {
				val = READ_LE_UINT16(ptrEnd + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
			} else if (cmd == 1) {
				val = READ_LE_UINT16(ptrEnd + 18) * 4;
				strncpy0(buf, GET_VARO_STR(val), 19);
			} else {
				val = READ_LE_UINT16(ptrEnd + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
				if (buf[0] == '-') {
					while (strlen(buf) - 1 < (uint32)ptrEnd[17]) {
						_vm->_util->insertStr("0", buf, 1);
					}
				} else {
					while (strlen(buf) - 1 < (uint32)ptrEnd[17]) {
						_vm->_util->insertStr("0", buf, 0);
					}
				}
				if (_vm->_global->_language == 2)
					_vm->_util->insertStr(".", buf, strlen(buf) + 1 - ptrEnd[17]);
				else
					_vm->_util->insertStr(",", buf, strlen(buf) + 1 - ptrEnd[17]);
			}
			memcpy(str + strPos, buf, strlen(buf));
			memset(mask, maskChar, strlen(buf));
			if (ptrEnd[17] & 0x80) {
				strPos2 = strPos + strlen(buf);
				strPos++;
				ptrEnd += 23;
				ptr++;
			} else {
				strPos += strlen(buf);
				if (ptr[1] != ' ') {
					if ((ptr[1] == 2) &&
							(((int16)READ_LE_UINT16(ptr + 4)) == _destSpriteY)) {
						ptr += 5;
						str[strPos] = ' ';
						mask[strPos++] = maskChar;
					}
				} else {
					str[strPos] = ' ';
					mask[strPos++] = maskChar;
					while (ptr[1] == ' ')
						ptr++;
					if ((ptr[1] == 2) &&
							(((int16)READ_LE_UINT16(ptr + 4)) == _destSpriteY))
						ptr += 5;
				}
				ptrEnd += 23;
				ptr++;
			}
			break;
		}
	}

	_renderFlags = savedFlags;
	if (!(_renderFlags & RENDERFLAG_COLLISIONS))
		return;

	_vm->_game->checkCollisions(0, 0, 0, 0);

	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

void Draw_v2::spriteOperation(int16 operation) {
	uint16 id;
	byte *dataBuf;
	int16 len;
	int16 x, y;
	SurfaceDescPtr sourceSurf, destSurf;
	bool deltaVeto;
	int16 left;
	int16 ratio;
	// Always assigned to -1 in Game::loadTotFile()
	int16 someHandle = -1;

	deltaVeto = (operation & 0x10) != 0;
	operation &= 0x0F;

	if (_sourceSurface >= 100)
		_sourceSurface -= 80;
	if (_destSurface >= 100)
		_destSurface -= 80;

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaVeto) {
		if ((_sourceSurface == 21) && (operation != DRAW_LOADSPRITE)) {
			_spriteLeft += _backDeltaX;
			_spriteTop += _backDeltaY;
		}

		if (_destSurface == 21) {
			_destSpriteX += _backDeltaX;
			_destSpriteY += _backDeltaY;
			if ((operation == DRAW_DRAWLINE) ||
			   ((operation >= DRAW_DRAWBAR) && (operation <= DRAW_FILLRECTABS))) {
				_spriteRight += _backDeltaX;
				_spriteBottom += _backDeltaY;
			}
		}
	}

	int16 spriteLeft = _spriteLeft;
	int16 spriteTop = _spriteTop;
	int16 spriteRight = _spriteRight;
	int16 spriteBottom = _spriteBottom;
	int16 destSpriteX = _destSpriteX;
	int16 destSpriteY = _destSpriteY;
	int16 destSurface = _destSurface;
	int16 sourceSurface = _sourceSurface;

	if (_vm->_video->_splitSurf && ((_destSurface == 20) || (_destSurface == 21))) {
		if ((_destSpriteY >= _vm->_video->_splitStart)) {
			_destSpriteY -= _vm->_video->_splitStart;
			if ((operation == DRAW_DRAWLINE) ||
				 ((operation >= DRAW_DRAWBAR) && (operation <= DRAW_FILLRECTABS)))
				_spriteBottom -= _vm->_video->_splitStart;

			_destSurface += 4;
		}

		if ((_spriteTop >= _vm->_video->_splitStart) && (operation == DRAW_BLITSURF)) {
			_spriteTop -= _vm->_video->_splitStart;
			if (_destSurface < 24)
				_destSurface += 4;
		}

	}

	adjustCoords(0, &_destSpriteX, &_destSpriteY);
	if ((operation != DRAW_LOADSPRITE) && (_needAdjust != 2)) {
		adjustCoords(0, &_spriteRight, &_spriteBottom);
		adjustCoords(0, &_spriteLeft, &_spriteTop);

		if (operation == DRAW_DRAWLINE) {
			if ((_spriteRight == _destSpriteX) || (_spriteBottom == _destSpriteY)) {
				operation = DRAW_FILLRECTABS;
				_backColor = _frontColor;
			}
		} else if (operation == DRAW_DRAWLETTER)
			operation = DRAW_BLITSURF;

		if (operation == DRAW_DRAWLINE) {
			if (_spriteBottom < _destSpriteY) {
				SWAP(_spriteBottom, _destSpriteY);
				SWAP(_spriteRight, _destSpriteX);
			}
		} else if ((operation == DRAW_LOADSPRITE) ||
		           (operation > DRAW_PRINTTEXT)) {
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

	if (!destSurf) {
		warning("Can't do operation %d on surface %d: nonexistent", operation, _destSurface);
		return;
	}

	switch (operation) {
	case DRAW_BLITSURF:
	case DRAW_DRAWLETTER:
		if (!sourceSurf || !destSurf)
			break;

		_vm->_video->drawSprite(*_spritesArray[_sourceSurface],
				*_spritesArray[_destSurface],
				_spriteLeft, spriteTop,
				_spriteLeft + _spriteRight - 1,
				_spriteTop + _spriteBottom - 1,
				_destSpriteX, _destSpriteY, _transparency);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_PUTPIXEL:
		_vm->_video->putPixel(_destSpriteX, _destSpriteY, _frontColor,
		                      *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _destSpriteX, _destSpriteY);
		break;

	case DRAW_FILLRECT:
		_vm->_video->fillRect(*_spritesArray[_destSurface], destSpriteX,
				_destSpriteY, _destSpriteX + _spriteRight - 1,
				_destSpriteY + _spriteBottom - 1, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_DRAWLINE:
		_vm->_video->drawLine(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_INVALIDATE:
		_vm->_video->drawCircle(*_spritesArray[_destSurface], _destSpriteX,
				_destSpriteY, _spriteRight, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX - _spriteRight, _destSpriteY - _spriteBottom,
				_destSpriteX + _spriteRight, _destSpriteY + _spriteBottom);
		break;

	case DRAW_LOADSPRITE:
		id = _spriteLeft;

		if ((id >= 30000) || (_vm->_game->_lomHandle >= 0)) {
			dataBuf = 0;

			if (_vm->_game->_lomHandle >= 0)
				warning("Urban Stub: LOADSPRITE %d, LOM", id);
			else
				dataBuf = _vm->_game->loadExtData(id, &_spriteRight, &_spriteBottom);

			if (!dataBuf)
				break;

			_vm->_video->drawPackedSprite(dataBuf,
					_spriteRight, _spriteBottom, _destSpriteX, _destSpriteY,
					_transparency, *_spritesArray[_destSurface]);

			dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
					_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);

			delete[] dataBuf;
			break;
		}

		// Load from .TOT resources
		if (!(dataBuf = _vm->_game->loadTotResource(id, 0, &_spriteRight, &_spriteBottom)))
			break;

		_vm->_video->drawPackedSprite(dataBuf,
		    _spriteRight, _spriteBottom,
		    _destSpriteX, _destSpriteY,
		    _transparency, *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_PRINTTEXT:
		len = strlen(_textToPrint);
		left = _destSpriteX;

		if ((_fontIndex >= 4) || (_fontToSprite[_fontIndex].sprite == -1)) {

			if (!_fonts[_fontIndex]->extraData) {
				if (((int8) _textToPrint[0]) == -1) {
					_vm->validateLanguage();

					dataBuf = _vm->_game->_totTextData->dataPtr + _textToPrint[1] + 1;
					len = *dataBuf++;
					for (int i = 0; i < len; i++, dataBuf += 2) {
						_vm->_video->drawLetter(READ_LE_UINT16(dataBuf), _destSpriteX,
								_destSpriteY, _fonts[_fontIndex], _transparency, _frontColor,
								_backColor, *_spritesArray[_destSurface]);
					}
				} else {
					drawString(_textToPrint, _destSpriteX, _destSpriteY, _frontColor,
							_backColor, _transparency, *_spritesArray[_destSurface],
							_fonts[_fontIndex]);
					_destSpriteX += len * _fonts[_fontIndex]->itemWidth;
				}
			} else {
				for (int i = 0; i < len; i++) {
					if ((someHandle < 0) || (_textToPrint[i] != ' ')) {
						_vm->_video->drawLetter(_textToPrint[i], _destSpriteX,
								_destSpriteY, _fonts[_fontIndex], _transparency,
								_frontColor, _backColor, *_spritesArray[_destSurface]);
						_destSpriteX += *(_fonts[_fontIndex]->extraData +
								(_textToPrint[i] - _fonts[_fontIndex]->startItem));
					}
					else
						_destSpriteX += _fonts[_fontIndex]->itemWidth;
				}
			}

		} else {
			sourceSurf = _spritesArray[_fontToSprite[_fontIndex].sprite];
			ratio = ((sourceSurf == _frontSurface) || (sourceSurf == _backSurface)) ?
				320 : sourceSurf->getWidth();
			ratio /= _fontToSprite[_fontIndex].width;
			for (int i = 0; i < len; i++) {
				y = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) / ratio)
					* _fontToSprite[_fontIndex].height;
				x = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) % ratio)
					* _fontToSprite[_fontIndex].width;
				_vm->_video->drawSprite(*_spritesArray[_fontToSprite[_fontIndex].sprite],
						*_spritesArray[_destSurface], x, y,
						x + _fontToSprite[_fontIndex].width - 1,
						y + _fontToSprite[_fontIndex].height - 1,
						_destSpriteX, _destSpriteY, _transparency);
				_destSpriteX += _fontToSprite[_fontIndex].width;
			}
		}

		dirtiedRect(_destSurface, left, _destSpriteY,
				_destSpriteX - 1, _destSpriteY + _fonts[_fontIndex]->itemHeight - 1);
		break;

	case DRAW_DRAWBAR:
		if (_needAdjust != 2) {
			_vm->_video->fillRect(*_spritesArray[_destSurface],
					_destSpriteX, _spriteBottom - 1,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->fillRect(*_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_destSpriteX + 1, _spriteBottom, _frontColor);

			_vm->_video->fillRect(*_spritesArray[_destSurface],
					_spriteRight - 1, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->fillRect(*_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY + 1, _frontColor);
		} else {
			_vm->_video->drawLine(*_spritesArray[_destSurface],
					_destSpriteX, _spriteBottom,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->drawLine(*_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_destSpriteX, _spriteBottom, _frontColor);

			_vm->_video->drawLine(*_spritesArray[_destSurface],
					_spriteRight, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_vm->_video->drawLine(*_spritesArray[_destSurface],
					_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY, _frontColor);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_CLEARRECT:
		if ((_backColor != 16) && (_backColor != 144)) {
			_vm->_video->fillRect(*_spritesArray[_destSurface],
			    _destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    _backColor);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_FILLRECTABS:
		_vm->_video->fillRect(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;
	}

	_spriteLeft = spriteLeft;
	_spriteTop = spriteTop;
	_spriteRight = spriteRight;
	_spriteBottom = spriteBottom;
	_destSpriteX = destSpriteX;
	_destSpriteY = destSpriteY;
	_destSurface = destSurface;
	_sourceSurface = sourceSurface;

	if (operation == DRAW_PRINTTEXT) {
		len = _fonts[_fontIndex]->itemWidth;
		adjustCoords(1, &len, 0);
		_destSpriteX += len * strlen(_textToPrint);
	}

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaVeto) {
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
