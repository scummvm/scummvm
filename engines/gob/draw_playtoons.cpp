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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "common/endian.h"

#include "gob/draw.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/hotspots.h"
#include "gob/resources.h"
#include "gob/scenery.h"
#include "gob/script.h"

namespace Gob {

Draw_Playtoons::Draw_Playtoons(GobEngine *vm) : Draw_v2(vm) {
}

void Draw_Playtoons::printTotText(int16 id) {
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

	_vm->validateLanguage();

	TextItem *textItem = _vm->_game->_resources->getTextItem(id);
	if (!textItem)
		return;

	size    = textItem->getSize();
	dataPtr = textItem->getData();
	ptr     = dataPtr;
	_pattern = 0;

	bool isSubtitle = (ptr[1] & 0x80) != 0;

	if (isSubtitle && !_vm->_global->_doSubtitles) {
		delete textItem;
		return;
	}

	if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
		destX = (READ_LE_UINT16(ptr) & 0x7FFF) * 2;
		spriteRight = READ_LE_UINT16(ptr + 4) * 2 + 1;
	} else {
		// No mask used for Fascination
		destX = READ_LE_UINT16(ptr);
		if (_vm->getGameType() != kGameTypeFascination)
			destX &= 0x7FFF;
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

	if (_renderFlags & RENDERFLAG_CAPTUREPUSH) {
		_vm->_game->capturePush(destX, destY,
				spriteRight - destX + 1, spriteBottom - destY + 1);
		(*_vm->_scenery->_pCaptureCounter)++;
	}

	_destSpriteX = destX;
	_destSpriteY = destY;
	_spriteRight = spriteRight;
	_spriteBottom = spriteBottom;
	_destSurface = kBackSurface;

	ptrEnd = ptr + 8;
	ptr += 9;

	if (*ptrEnd == 16)
		_backColor = -1;
	else
		_backColor = *ptr + _colorOffset;

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
			_frontColor = (cmd & 0xF) + _colorOffset;
			spriteOperation(DRAW_DRAWLINE);
			break;
		case 1:
			_frontColor = (cmd & 0xF) + _colorOffset;
			spriteOperation(DRAW_DRAWBAR);
			break;
		case 2:
			_backColor = (cmd & 0xF) + _colorOffset;
			spriteOperation(DRAW_FILLRECTABS);
			break;
		default:
			break;
		}
	}
	ptr += 2;

	ptrEnd = ptr;
	while (((ptrEnd - dataPtr) < size) && (*ptrEnd != 1)) {
		// Converting to unknown commands/characters to spaces
		if ((_vm->_game->_script->getVersionMinor() < 2) && (*ptrEnd > 3) && (*ptrEnd < ' '))
			*ptrEnd = ' ';

		switch (*ptrEnd) {
		case 1:
			break;

		case 2:
		case 5:
		case 12:
			ptrEnd += 5;
			break;

		case 3:
		case 4:
		case 15:
			ptrEnd += 2;
			break;

		case 6:
			ptrEnd++;
			switch (*ptrEnd & 0xC0) {
			case 0:
				ptrEnd++;
				break;
			case 0x40:
				ptrEnd += 9;
				break;
			case 0x80:
				ptrEnd += 3;
				break;
			case 0xC0:
				ptrEnd += 11;
				break;
			}
			break;

		case 10:
			ptrEnd += (ptrEnd[1] * 2) + 2;
			break;

		case 11:
			do { ptrEnd++; } while (*ptrEnd != 0);
			ptrEnd += 2;
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
	char mask[200], str[200], buf[50];

	memset(mask, 0, 200);
	memset(str, ' ', 200);
	_backColor = 0;
	_transparency = 1;

	Common::Array<TotTextInfo> totTextInfos;
	if (_vm->_game->_script->getVersionMinor() >= 4)
		totTextInfos.resize(32);

#ifdef USE_TTS
	Common::String ttsMessage;
#endif
	while (true) {
		if ((((*ptr >= 1) && (*ptr <= 7)) || (*ptr == 10)) && (strPos != 0)) {
			str[MAX(strPos, strPos2)] = 0;
			strPosBak = strPos;
			width = stringLength(str, fontIndex);
			adjustCoords(1, &width, nullptr);

			if (colCmd & 0x0F) {
				rectLeft = offX - 2;
				rectTop = offY - 2;
				rectRight = offX + width + 1;
				rectBottom = _fonts[fontIndex]->getCharHeight();
				adjustCoords(1, &rectBottom, nullptr);
				rectBottom += offY + 1;
				adjustCoords(0, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);

				if (_vm->_game->_script->getVersionMinor() < 4 || colId < 1 || colId > 32) {
					if (colId != -1) {
						uint16 hotspotIdOff = (colCmd & 0x80) ? 0xD000 : 0x4000;
						_vm->_game->_hotspots->add(colId + hotspotIdOff, rectLeft, rectTop,
												   rectRight, rectBottom, (uint16) Hotspots::kTypeClick, 0, 0, 0, 0);
					}

					if (_needAdjust != 2 && _needAdjust < 10)
						drawButton(colCmd & 0x0F, rectLeft + 4, rectTop + 4,
								   rectRight - 4, rectBottom - 4, str, fontIndex, frontColor, colId);
					else
						drawButton(colCmd & 0x0F, rectLeft + 2, rectTop + 2,
								   rectRight - 2, rectBottom - 2, str, fontIndex, frontColor, colId);
				} else if (totTextInfos[colId - 1].str.empty()) {
					totTextInfos[colId - 1].str = str;
					totTextInfos[colId - 1].rectLeft = rectLeft;
					totTextInfos[colId - 1].rectTop = rectTop;
					totTextInfos[colId - 1].rectRight = rectRight;
					totTextInfos[colId - 1].rectBottom = rectBottom;
					totTextInfos[colId - 1].colCmd = colCmd;
					totTextInfos[colId - 1].fontIndex = fontIndex;
					totTextInfos[colId - 1].color = frontColor;
				} else {
					if (rectLeft < totTextInfos[colId - 1].rectLeft)
						totTextInfos[colId - 1].rectLeft = rectLeft;

					if (rectTop < totTextInfos[colId - 1].rectTop)
						totTextInfos[colId - 1].rectTop = rectTop;

					if (rectRight > totTextInfos[colId - 1].rectRight)
						totTextInfos[colId - 1].rectRight = rectRight;

					if (rectBottom > totTextInfos[colId - 1].rectBottom)
						totTextInfos[colId - 1].rectBottom = rectBottom;

					totTextInfos[colId - 1].str += '\\';
					totTextInfos[colId - 1].str += str;
				}
			} else {
				_destSpriteX = offX;
				_destSpriteY = offY;
				_fontIndex   = fontIndex;
				_frontColor  = frontColor + _colorOffset;
				_textToPrint = str;
#ifdef USE_TTS
				ttsMessage += _textToPrint;
				ttsMessage += " ";
#endif

				if (isSubtitle) {
					_fontIndex  = _subtitleFont;
					_frontColor = _subtitleColor;
				}

				if (_needAdjust != 2) {
					if ((_destSpriteX >= destX) && (_destSpriteY >= destY) &&
					    (((_fonts[_fontIndex]->getCharHeight() / 2) + _destSpriteY - 1) <= spriteBottom)) {
						while (((_destSpriteX + width - 1) > spriteRight) && (width > 0)) {
							width -= _fonts[_fontIndex]->getCharWidth() / 2;
							str[strlen(str) - 1] = '\0';
						}
						spriteOperation(DRAW_PRINTTEXT, false);
					}
				} else
					spriteOperation(DRAW_PRINTTEXT, false);

				width = strlen(str);
				rectLeft = offX;
				for (strPos = 0; strPos < width; strPos++) {
					char charStrAtPos[2];
					charStrAtPos[0] = str[strPos];
					charStrAtPos[1] = '\0';

					int16 charWidth = stringLength(charStrAtPos, fontIndex);
					adjustCoords(1, &charWidth, nullptr);

					if (mask[strPos] == '\0') {
						rectLeft += charWidth;
						continue;
					}

					_destSpriteX = rectLeft;
					rectLeft += charWidth;

					rectTop = _fonts[fontIndex]->getCharHeight();
					adjustCoords(1, nullptr, &rectTop);

					_spriteRight = _destSpriteX + rectLeft - 1;
					_spriteBottom = offY + rectTop;
					_destSpriteY = _spriteBottom;
					spriteOperation(DRAW_DRAWLINE);
				}
			}

			rectLeft = 0;
			if (_vm->_game->_script->getVersionMinor() < 4) {
				rectLeft += strPosBak * _fonts[_fontIndex]->getCharWidth();
			} else {
				rectLeft += stringLength(str, fontIndex);
			}

			adjustCoords(1, &rectLeft, nullptr);
			offX += rectLeft;
			strPos = 0;
			strPos2 = -1;
			memset(mask, 0, 200);
			memset(str, ' ', 200);
		}

		if (*ptr == 1)
			break;

		cmd = *ptr;

		switch ((uint8) cmd) {
		case 2:
		case 5: // Set dest coordinates
			ptr++;
			offX = destX + (int16)READ_LE_UINT16(ptr);
			offY = destY + (int16)READ_LE_UINT16(ptr + 2);
			if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
				offX += (int16)READ_LE_UINT16(ptr);
				offY += (int16)READ_LE_UINT16(ptr + 2);
			}

			ptr += 4;
			break;

		case 3: // Set font and color
			ptr++;
			fontIndex = ((*ptr & 0xF0) >> 4) & 7;
			frontColor = *ptr & 0x0F;
			ptr++;

			if (isSubtitle) {
				_subtitleFont  = fontIndex;
				_subtitleColor = frontColor;
			}
			break;

		case 4:
			ptr++;
			frontColor = *ptr++;

			if (isSubtitle)
				_subtitleColor = frontColor;
			break;

		case 6:
			colCmd = ptr[1];
			colId = -1;
			if ((colCmd & 0x80) || (colCmd & 0x20)) {
				colId = (int16)READ_LE_UINT16(ptr + 2);
				ptrEnd = ptr + 4;
			}
			ptr = ptrEnd;

			if (colCmd & 0x40) {
				rectLeft = destX + (int16)READ_LE_UINT16(ptr);
				rectRight = destX + (int16)READ_LE_UINT16(ptr + 2);
				rectTop = destY + (int16)READ_LE_UINT16(ptr + 4);
				rectBottom = destY + (int16)READ_LE_UINT16(ptr + 6);
				adjustCoords(2, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);
				_vm->_game->_hotspots->add(colId + 0x0D000, rectLeft, rectTop,
						rectRight, rectBottom, (uint16) Hotspots::kTypeClick, 0, 0, 0, 0);
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
			WRITE_LE_UINT16(str + 1, ptr - _vm->_game->_resources->getTexts());
			str[3] = 0;
			ptr++;
			for (int i = *ptr++; i > 0; i--) {
				mask[strPos++] = maskChar;
				ptr += 2;
			}
			break;

		case 11:
			do {
				ptrEnd = ptr;
				ptr++;
			} while (*ptr);
			break;

		case 12:
			warning("STUB: DrawPlaytoons:printTotText, case 12");
			break;

		case 13:
		case 14:
			ptr++;
			break;

		case 15:
			ptr += 2;
			break;

		default:
			if (*ptr == 186 && _vm->_game->_script->getVersionMinor() < 3) {
				cmd = ptrEnd[17] & 0x7F;
				if (cmd == 0) {
					val = READ_LE_UINT16(ptrEnd + 18) * 4;
					Common::sprintf_s(buf, "%d", (int32)VAR_OFFSET(val));
				} else if (cmd == 1) {
					val = READ_LE_UINT16(ptrEnd + 18) * 4;
					Common::strlcpy(buf, GET_VARO_STR(val), 20);
				} else {
					val = READ_LE_UINT16(ptrEnd + 18) * 4;
					Common::sprintf_s(buf, "%d", (int32)VAR_OFFSET(val));
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
			} else {
				if (*ptr != ' ') {
					str[strPos] = *ptr;
				}

				mask[strPos] = maskChar;
				strPos++;
			}

			ptr++;
		}
	}

	ptrEnd = ptr + 2;
	if (_vm->_game->_script->getVersionMinor() >= 4 && ptr + 4 < dataPtr + size) {
		int16 cmd2 = *ptrEnd;
		if (cmd2 & 0x80)
			cmd = (cmd2 & 0x7F) * 24;
		else
			cmd = (cmd2 & 0x7F) * 22;

		ptr += cmd + 3;
		int16 len = *ptr++;
		if (len > 0)
			warning("STUB: DrawPlaytoons:printTotText, len=%d", len);
		ptrEnd = ptr;
		for (strPos = 0; strPos < len; strPos++) {
			byte *ptr2 = ptr + strPos * 0x4C;
			int16 textIndex = (*ptr2 + 0x4A);
			if (textIndex) {
				warning("STUB: DrawPlaytoons:printTotText, strPos = %d, update tot text %d", strPos, textIndex);
			} else {
				warning("STUB: DrawPlaytoons:printTotText, strPos = %d, BLITSURF case", strPos);
			}
		}
	}

	ptr = ptrEnd;
	if (_vm->_game->_script->getVersionMinor() >= 4) {
		for (strPos = 0; strPos < (int16)totTextInfos.size(); strPos++) {
			TotTextInfo &totTextInfo = totTextInfos[strPos];
			if (totTextInfo.str.empty())
				continue;

			int hotspotIdOff = (totTextInfo.colCmd & 0x80) ? 0xD001 : 0x4001;
			_vm->_game->_hotspots->add(strPos + hotspotIdOff, totTextInfo.rectLeft, totTextInfo.rectTop,
									   totTextInfo.rectRight, totTextInfo.rectBottom, Hotspots::kTypeClick, 0, 0, 0, 0);

			if (_needAdjust != 2 && _needAdjust < 10)
				drawButton(totTextInfo.colCmd & 0x0F, totTextInfo.rectLeft + 4, totTextInfo.rectTop + 4,
						   totTextInfo.rectRight - 4, totTextInfo.rectBottom - 4, totTextInfo.str.begin(),
						   totTextInfo.fontIndex, totTextInfo.color, strPos + 1);
			else
				drawButton(totTextInfo.colCmd & 0x0F, totTextInfo.rectLeft + 2, totTextInfo.rectTop + 2,
						   totTextInfo.rectRight - 2, totTextInfo.rectBottom - 2, totTextInfo.str.begin(),
						   totTextInfo.fontIndex, totTextInfo.color, strPos + 1);
		}
	}

#ifdef USE_TTS
	if (_previousTot != ttsMessage && !isSubtitle) {
		if (_vm->_game->_hotspots->hoveringOverHotspot()) {
			_vm->sayText(ttsMessage);
		} else {
			_vm->sayText(ttsMessage, Common::TextToSpeechManager::QUEUE);
		}

		_previousTot = ttsMessage;
	}
#endif

	delete textItem;
	_renderFlags = savedFlags;

	if (!(_renderFlags & RENDERFLAG_COLLISIONS))
		return;

	_vm->_game->_hotspots->check(0, 0);

	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

void Draw_Playtoons::spriteOperation(int16 operation, bool ttsAddHotspotText) {
	int16 len;
	int16 x, y;
	bool deltaVeto;
	int16 left;
	int16 ratio;
	Resource *resource;

	deltaVeto = (operation & 0x10) != 0;
	operation &= 0x0F;

	if (_sourceSurface >= 100)
		_sourceSurface -= 80;
	if (_destSurface >= 100)
		_destSurface -= 80;

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaVeto) {
		if ((_sourceSurface == kBackSurface) && (operation != DRAW_LOADSPRITE)) {
			_spriteLeft += _backDeltaX;
			_spriteTop += _backDeltaY;
		}

		if (_destSurface == kBackSurface) {
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

	if (_vm->_video->_splitSurf && ((_destSurface == kFrontSurface) || (_destSurface == kBackSurface))) {
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

	if (!_spritesArray[_destSurface]) {
		warning("Can't do operation %d on surface %d: nonexistent", operation, _destSurface);
		return;
	}

	switch (operation) {
	case DRAW_BLITSURF:
	case DRAW_DRAWLETTER:
		if (!_spritesArray[_sourceSurface] || !_spritesArray[_destSurface])
			break;

		if (_transparency & 0x200) {
			uint8 strength = 16 - (((uint16) _transparency) >> 12);
			_spritesArray[_destSurface]->blitShaded(*_spritesArray[_sourceSurface],
											  _spriteLeft, spriteTop,
											  _spriteLeft + _spriteRight - 1,
											  _spriteTop + _spriteBottom - 1,
											  _destSpriteX, _destSpriteY,
											  strength,
											  0,
											  _vm->getPixelFormat());
		} else {
			_spritesArray[_destSurface]->blit(*_spritesArray[_sourceSurface],
											  _spriteLeft, spriteTop,
											  _spriteLeft + _spriteRight - 1,
											  _spriteTop + _spriteBottom - 1,
											  _destSpriteX, _destSpriteY, (_transparency == 0) ? -1 : 0);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_PUTPIXEL:
		switch (_pattern & 0xFF) {
		case 0xFF:
			WRITE_VAR(0, _spritesArray[_destSurface]->get(_destSpriteX, _destSpriteY).get());
			break;
		case 1:
			_spritesArray[_destSurface]->fillRect(destSpriteX,
					_destSpriteY, _destSpriteX + 1,
					_destSpriteY + 1, _frontColor);
			break;
		case 2:
			_spritesArray[_destSurface]->fillRect(destSpriteX - 1,
					_destSpriteY - 1, _destSpriteX + 1,
					_destSpriteY + 1, _frontColor);
			break;
		case 3:
			_spritesArray[_destSurface]->fillRect(destSpriteX - 1,
					_destSpriteY - 1, _destSpriteX + 2,
					_destSpriteY + 2, _frontColor);
			break;
		default:
			_spritesArray[_destSurface]->putPixel(_destSpriteX, _destSpriteY, _frontColor);
			break;
		}
		dirtiedRect(_destSurface, _destSpriteX - (_pattern / 2),
			                      _destSpriteY - (_pattern / 2),
								  _destSpriteX + (_pattern + 1) / 2,
								  _destSpriteY + (_pattern + 1) / 2);
		break;
	case DRAW_FILLRECT:
		switch (_pattern & 0xFF) {
		case 3:
		case 4:
			warning("oPlaytoons_spriteOperation: operation DRAW_FILLRECT, pattern %d", _pattern & 0xFF);
			break;

		case 1: {
			_spritesArray[_destSurface]->fillArea(destSpriteX,
												  _destSpriteY,
												  _destSpriteX + _spriteRight - 1,
												  _destSpriteY + _spriteBottom - 1,
												  _backColor & 0xFF,
												  (_backColor >> 8) & 0xFF);

			dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
						_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
			break;
		}
		case 2: {
			Common::Rect dirtyRect = _spritesArray[_destSurface]->fillAreaAtPoint(destSpriteX,
																				  _destSpriteY,
																				  _backColor);
			dirtiedRect(_destSurface, dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
			break ;
		}
		case 0: {
			if (!(_backColor & 0xFF00) || !(_backColor & 0x0100)) {
				_spritesArray[_destSurface]->fillRect(_destSpriteX,
													  _destSpriteY, _destSpriteX + _spriteRight - 1,
													  _destSpriteY + _spriteBottom - 1, _backColor);
			} else {
				uint8 strength = 16 - (((uint16) _backColor) >> 12);

				_spritesArray[_destSurface]->shadeRect(_destSpriteX,
													   _destSpriteY, _destSpriteX + _spriteRight - 1,
													   _destSpriteY + _spriteBottom - 1, _backColor, strength);
			}

			dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
						_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
			break;
		}
		default:
			warning("oPlaytoons_spriteOperation: operation DRAW_FILLRECT, unexpected pattern %d", _pattern & 0xFF);
			break;
		}
		break;

	case DRAW_DRAWLINE:
		if ((_needAdjust != 2) && (_needAdjust < 10)) {
			warning ("oPlaytoons_spriteOperation: operation DRAW_DRAWLINE, draw multiple lines");
				_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);
				_spritesArray[_destSurface]->drawLine(_destSpriteX + 1, _destSpriteY,
					_spriteRight + 1, _spriteBottom, _frontColor);
				_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY + 1,
					_spriteRight, _spriteBottom + 1, _frontColor);
				_spritesArray[_destSurface]->drawLine(_destSpriteX + 1, _destSpriteY + 1,
					_spriteRight + 1, _spriteBottom + 1, _frontColor);
		} else {
			switch (_pattern & 0xFF) {
			case 0:
				_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

				break;
			default:
				for (int16 i = 0; i <= _pattern; i++)
					for (int16 j = 0; j <= _pattern; j++)
						_spritesArray[_destSurface]->drawLine(
								_destSpriteX  - (_pattern / 2) + i,
								_destSpriteY  - (_pattern / 2) + j,
								_spriteRight  - (_pattern / 2) + i,
								_spriteBottom - (_pattern / 2) + j,
								_frontColor);
				break;
			}
		}
		dirtiedRect(_destSurface, MIN(_destSpriteX, _spriteRight)  - _pattern,
								  MIN(_destSpriteY, _spriteBottom) - _pattern,
								  MAX(_destSpriteX, _spriteRight)  + _pattern + 1,
								  MAX(_destSpriteY, _spriteBottom) + _pattern + 1);
		break;

	case DRAW_INVALIDATE:
		_spritesArray[_destSurface]->drawCircle(_destSpriteX,
												_destSpriteY, _spriteRight, _frontColor, _pattern & 0xFF);

		dirtiedRect(_destSurface, _destSpriteX - _spriteRight, _destSpriteY - _spriteBottom,
				_destSpriteX + _spriteRight, _destSpriteY + _spriteBottom);
		break;

	case DRAW_LOADSPRITE:
		resource = _vm->_game->_resources->getResource((uint16) _spriteLeft,
			                                             &_spriteRight, &_spriteBottom);

		if (!resource)
			break;

		_vm->_video->drawPackedSprite(resource->getData(),
				_spriteRight, _spriteBottom, _destSpriteX, _destSpriteY,
				_transparency, *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);

		delete resource;
		break;

	case DRAW_PRINTTEXT:
		len = strlen(_textToPrint);
		left = _destSpriteX;

		if ((_fontIndex >= 4) || (_fontToSprite[_fontIndex].sprite == -1)) {
			Font *font = _fonts[_fontIndex];
			if (!font) {
				warning("oPlaytoons_spriteOperation: Trying to print \"%s\" with undefined font %d", _textToPrint, _fontIndex);
				break;
			}

			if (font->isMonospaced()) {
				if (((int8) _textToPrint[0]) == -1) {
					_vm->validateLanguage();

					byte *dataBuf = _vm->_game->_resources->getTexts() + _textToPrint[1] + 1;
					len = *dataBuf++;
					for (int i = 0; i < len; i++, dataBuf += 2) {
						font->drawLetter(*_spritesArray[_destSurface], READ_LE_UINT16(dataBuf),
								_destSpriteX, _destSpriteY, _frontColor, _backColor, _transparency);
					}
				} else {
					font->drawString(_textToPrint, _destSpriteX, _destSpriteY, _frontColor,
							_backColor, _transparency, *_spritesArray[_destSurface]);
					_destSpriteX += len * font->getCharWidth();
				}
			} else {
				for (int i = 0; i < len; i++) {
					font->drawLetter(*_spritesArray[_destSurface], _textToPrint[i],
							_destSpriteX, _destSpriteY, _frontColor, _backColor, _transparency);
					_destSpriteX += font->getCharWidth(_textToPrint[i]);
				}
			}

		} else {
			SurfacePtr sourceSurf = _spritesArray[_fontToSprite[_fontIndex].sprite];
			ratio = ((sourceSurf == _frontSurface) || (sourceSurf == _backSurface)) ?
				320 : sourceSurf->getWidth();
			ratio /= _fontToSprite[_fontIndex].width;
			for (int i = 0; i < len; i++) {
				y = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) / ratio)
					* _fontToSprite[_fontIndex].height;
				x = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) % ratio)
					* _fontToSprite[_fontIndex].width;
				_spritesArray[_destSurface]->blit(*_spritesArray[_fontToSprite[_fontIndex].sprite], x, y,
						x + _fontToSprite[_fontIndex].width - 1,
						y + _fontToSprite[_fontIndex].height - 1,
						_destSpriteX, _destSpriteY, (_transparency == 0) ? -1 : 0);
				_destSpriteX += _fontToSprite[_fontIndex].width;
			}
		}

#ifdef USE_TTS
		if (ttsAddHotspotText) {
			_vm->_game->_hotspots->addHotspotTTSText(_textToPrint, left, _destSpriteY, 
											_destSpriteX - 1, _destSpriteY + _fonts[_fontIndex]->getCharHeight() - 1, _destSurface);
		}
#endif

		dirtiedRect(_destSurface, left, _destSpriteY,
				_destSpriteX - 1, _destSpriteY + _fonts[_fontIndex]->getCharHeight() - 1);
		break;

	case DRAW_DRAWBAR:
		if ((_needAdjust != 2) && (_needAdjust < 10)){
			_spritesArray[_destSurface]->fillRect(_destSpriteX, _spriteBottom - 1,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
					_destSpriteX + 1, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->fillRect(_spriteRight - 1, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY + 1, _frontColor);
		} else {
			_spritesArray[_destSurface]->drawLine(_destSpriteX, _spriteBottom,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_destSpriteX, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->drawLine(_spriteRight, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY, _frontColor);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_CLEARRECT:
		warning ("oPlaytoons_spriteOperation: DRAW_CLEARRECT uses _backColor %d", _backColor);
		if (_backColor != -1) {
			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    _backColor);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_FILLRECTABS:
		_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	default:
		warning ("oPlaytoons_spriteOperation: Unhandled operation %d", operation);
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
		len = _fonts[_fontIndex]->getCharWidth();
		adjustCoords(1, &len, nullptr);
		_destSpriteX += len * strlen(_textToPrint);
	}

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaVeto) {
		if (_sourceSurface == kBackSurface) {
			_spriteLeft -= _backDeltaX;
			_spriteTop -= _backDeltaY;
		}

		if (_destSurface == kBackSurface) {
			_destSpriteX -= _backDeltaX;
			_destSpriteY -= _backDeltaY;
		}
	}
}

} // End of namespace Gob
