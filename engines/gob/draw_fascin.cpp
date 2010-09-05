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

//#include "common/endian.h"
//#include "graphics/cursorman.h"

#include "gob/draw.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/resources.h"

namespace Gob {

Draw_Fascination::Draw_Fascination(GobEngine *vm) : Draw_v2(vm) {
}

void Draw_Fascination::spriteOperation(int16 operation) {
	int16 len;
	int16 x, y;
	SurfaceDescPtr sourceSurf, destSurf;
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

	if (_renderFlags & 0x20) {
		if (_destSurface == kBackSurface || (operation == 0 && _sourceSurface == kBackSurface)) {
			drawWin(operation);
			return;
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

	sourceSurf = _spritesArray[_sourceSurface];
	destSurf = _spritesArray[_destSurface];

	if (!destSurf) {
		warning("Can't do operation %d on surface %d: nonexistent", operation, _destSurface);
		return;
	}

	switch (operation) {
	case DRAW_DRAWLETTER:
	case DRAW_BLITSURF:
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
				warning("Trying to print \"%s\" with undefined font %d", _textToPrint, _fontIndex);
				break;
			}

			if (font->isMonospaced()) {
				if (((int8) _textToPrint[0]) == -1) {
					_vm->validateLanguage();

					byte *dataBuf = _vm->_game->_resources->getTexts() + _textToPrint[1] + 1;
					len = *dataBuf++;
					for (int i = 0; i < len; i++, dataBuf += 2) {
						_vm->_video->drawLetter(READ_LE_UINT16(dataBuf), _destSpriteX,
								_destSpriteY, *font, _transparency, _frontColor,
								_backColor, *_spritesArray[_destSurface]);
					}
				} else {
					drawString(_textToPrint, _destSpriteX, _destSpriteY, _frontColor,
							_backColor, _transparency, *_spritesArray[_destSurface], *font);
					_destSpriteX += len * font->getCharWidth();
				}
			} else {
				for (int i = 0; i < len; i++) {
					_vm->_video->drawLetter(_textToPrint[i], _destSpriteX,
							_destSpriteY, *font, _transparency,
							_frontColor, _backColor, *_spritesArray[_destSurface]);
					_destSpriteX += font->getCharWidth(_textToPrint[i]);
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
				_destSpriteX - 1, _destSpriteY + _fonts[_fontIndex]->getCharHeight() - 1);
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
		len = _fonts[_fontIndex]->getCharWidth();
		adjustCoords(1, &len, 0);
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

void Draw_Fascination::drawWin(int16 fct) {
	int16 left;
	int16 top;
	int16 width;
	int16 height;

	bool found = false;
	int len;
	Resource *resource;
	int table[10];
	SurfaceDescPtr tempSrf;

	if (_destSurface == kBackSurface) {

		if (_vm->_global->_curWinId) {
			if (_fascinWin[_vm->_global->_curWinId].id == -1)
				return;

			else {
				_destSpriteX += _fascinWin[_vm->_global->_curWinId].left;
				_destSpriteY += _fascinWin[_vm->_global->_curWinId].top;
				if (fct == 3 || (fct >= 7 && fct <= 9)) {
					_spriteRight  += _fascinWin[_vm->_global->_curWinId].left;
					_spriteBottom += _fascinWin[_vm->_global->_curWinId].top;
				}
			}
		}

		left = _destSpriteX;
		top  = _destSpriteY;

	} else {
		if (_vm->_global->_curWinId) {
			if (_fascinWin[_vm->_global->_curWinId].id == -1)
				return;
			else {
				_spriteLeft += _fascinWin[_vm->_global->_curWinId].left;
				_spriteTop  += _fascinWin[_vm->_global->_curWinId].top;
			}
		}

		left = _spriteLeft;
		top  = _spriteTop;
	}

	for (int i = 0; i < 10; i++)
		table[i] = 0;

	switch (fct) {
	case DRAW_BLITSURF:    // 0 - move
	case DRAW_FILLRECT:    // 2 - fill rectangle
		width  = left + _spriteRight  - 1;
		height = top  + _spriteBottom - 1;
		break;

	case DRAW_PUTPIXEL:    // 1 - put a pixel
		width  = _destSpriteX;
		height = _destSpriteY;
		break;

	case DRAW_DRAWLINE:    // 3 - draw line
	case DRAW_DRAWBAR:     // 7 - draw border
	case DRAW_CLEARRECT:   // 8 - clear rectangle
	case DRAW_FILLRECTABS: // 9 - fill rectangle, with other coordinates
		width  = _spriteRight;
		height = _spriteBottom;
		break;

	case DRAW_INVALIDATE:  // 4 - Draw a circle
		left   = _destSpriteX - _spriteRight;
		top    = _destSpriteY - _spriteRight;
		width  = _destSpriteX + _spriteRight;
		height = _destSpriteY + _spriteBottom;
		break;

	case DRAW_LOADSPRITE:  // 5 - Uncompress and load a sprite
		// TODO: check the implementation, currently dirty cut and paste of DRAW_SPRITE code
		resource = _vm->_game->_resources->getResource((_spriteLeft & 0x3FFF),
				&_spriteRight, &_spriteBottom);

		if (!resource) {
			width  = 0;
			height = 0;
			break;
		}

		_vm->_video->drawPackedSprite(resource->getData(),
				_spriteRight, _spriteBottom, _destSpriteX, _destSpriteY,
				_transparency, *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);

		delete resource;

		width  = _destSpriteX + _spriteRight  - 1;
		height = _destSpriteY + _spriteBottom - 1;
		break;

	case DRAW_PRINTTEXT:  // 6 - Display string
		width  = _destSpriteX - 1 + strlen(_textToPrint) * _fonts[_fontIndex]->getCharWidth();
		height = _destSpriteY - 1 + _fonts[_fontIndex]->getCharHeight();
		break;

	case DRAW_DRAWLETTER: // 10 - Display a character
		if (_fontToSprite[_fontIndex].sprite == -1) {
			width  = _destSpriteX - 1 + _fonts[_fontIndex]->getCharWidth();
			height = _destSpriteY - 1 + _fonts[_fontIndex]->getCharHeight();
		} else {
			width  = _destSpriteX + _fontToSprite[_fontIndex].width  - 1;
			height = _destSpriteY + _fontToSprite[_fontIndex].height - 1;
		}
		break;

	default:
		error("winDraw - Unexpected fct value %d", fct);
		break;
	}

	for (int i = 0; i < 10; i++) {
		if ((i != _vm->_global->_curWinId) && (_fascinWin[i].id != -1)) {
			if (!_vm->_global->_curWinId || _fascinWin[i].id>_fascinWin[_vm->_global->_curWinId].id) {
				if ((_fascinWin[i].left + _fascinWin[i].width  > left) && (width  >= _fascinWin[i].left) &&
				    (_fascinWin[i].top  + _fascinWin[i].height > top ) && (height >= _fascinWin[i].top)) {
					found = true;
					table[_fascinWin[i].id] = i;
				}
			}
		}
	}

	if ((_sourceSurface == kBackSurface) && (fct == 0)) {
		_vm->_video->drawSprite(*_spritesArray[_sourceSurface], *_spritesArray[_destSurface],
								_spriteLeft, _spriteTop, _spriteLeft + _spriteRight - 1,
								_spriteTop + _spriteBottom - 1, _destSpriteX, _destSpriteY, _transparency);
		if (!found)
			return;

		int j = 0;
		if (_vm->_global->_curWinId != 0)
			j = _fascinWin[_vm->_global->_curWinId].id + 1;

		for (int i = 9; i >= j; i--) {
			if (table[i])
				_vm->_video->drawSprite(*_fascinWin[table[i]].savedSurface, *_spritesArray[_destSurface],
										 _fascinWin[table[i]].left & 7, 0,
										(_fascinWin[table[i]].left & 7) + _fascinWin[table[i]].width - 1,
										 _fascinWin[table[i]].height - 1, _fascinWin[table[i]].left - _spriteLeft + _destSpriteX,
										 _fascinWin[table[i]].top - _spriteTop + _destSpriteY, 0);
		}
		return;
	}

	if (found) {
		tempSrf = _vm->_video->initSurfDesc(_vm->_global->_videoMode, width - left + 1, height - top + 1, 0);
		_vm->_video->drawSprite(*_backSurface, *tempSrf, left, top, width, height, 0, 0, 0);

		int max = 0;
		if (_vm->_global->_curWinId != 0)
			max = _fascinWin[_vm->_global->_curWinId].id + 1;

		for (int i = 9; i >= max; i--) {
			if (table[i])
				_vm->_video->drawSprite(*_fascinWin[table[i]].savedSurface, *tempSrf,
										 _fascinWin[table[i]].left & 7, 0,
										(_fascinWin[table[i]].left & 7) + _fascinWin[table[i]].width - 1,
										 _fascinWin[table[i]].height - 1,
										 _fascinWin[table[i]].left  - left,
										 _fascinWin[table[i]].top   - top , 0);
		}

		invalidateRect(left, top, width, height);

		switch (fct) {
		case DRAW_BLITSURF:    // 0 - move
			_vm->_video->drawSprite(*_spritesArray[_sourceSurface], *tempSrf,
									_spriteLeft, _spriteTop, _spriteLeft + _spriteRight - 1,
									_spriteTop + _spriteBottom - 1, 0, 0, _transparency);
			break;

		case DRAW_PUTPIXEL:    // 1 - put a pixel
			_vm->_video->putPixel(0, 0, _frontColor, *tempSrf);
			break;

		case DRAW_FILLRECT:    // 2 - fill rectangle
			_vm->_video->fillRect(*tempSrf, 0, 0, _spriteRight - 1, _spriteBottom - 1, _backColor);
			break;

		case DRAW_DRAWLINE:    // 3 - draw line
			_vm->_video->drawLine(*tempSrf, 0, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _frontColor);
			break;

		case DRAW_INVALIDATE:  // 4 - Draw a circle
			_vm->_video->drawCircle(*tempSrf, _spriteRight, _spriteRight, _spriteRight, _frontColor);
			break;

		case DRAW_LOADSPRITE:  // 5 - Uncompress and load a sprite
			decompWin(0, 0, tempSrf);
			break;

		case DRAW_PRINTTEXT:   // 6 - Display string
			len = strlen(_textToPrint);
			for (int j = 0; j < len; j++)
				_vm->_video->drawLetter(_textToPrint[j], j * _fonts[_fontIndex]->getCharWidth(), 0,
										*_fonts[_fontIndex], _transparency, _frontColor, _backColor, *tempSrf);
			_destSpriteX += len * _fonts[_fontIndex]->getCharWidth();
			break;

		case DRAW_DRAWBAR:     // 7 - draw border
			_vm->_video->drawLine(*tempSrf, 0, _spriteBottom - _destSpriteY, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _frontColor);
			_vm->_video->drawLine(*tempSrf, 0, 0, 0, _spriteBottom - _destSpriteY, _frontColor);
			_vm->_video->drawLine(*tempSrf, _spriteRight - _destSpriteX, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _frontColor);
			_vm->_video->drawLine(*tempSrf, 0, 0, _spriteRight - _destSpriteX, 0, _frontColor);
			break;

		case DRAW_CLEARRECT:   // 8 - clear rectangle
			if (_backColor < 16)
				_vm->_video->fillRect(*tempSrf, 0, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _backColor);
			break;

		case DRAW_FILLRECTABS: // 9 - fill rectangle, with other coordinates
			_vm->_video->fillRect(*tempSrf, 0, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _backColor);
			break;

		case DRAW_DRAWLETTER:  // 10 - Display a character
			if (_fontToSprite[_fontIndex].sprite == -1) {

				if (_letterToPrint)
					_vm->_video->drawLetter(_letterToPrint, 0, 0, *_fonts[_fontIndex], _transparency, _frontColor, _backColor, *tempSrf);
			} else {
				int xx, yy, nn;
				nn = _spritesArray[_fontToSprite[_fontIndex].sprite]->getWidth() / _fontToSprite[_fontIndex].width;
				yy = ((_letterToPrint - _fontToSprite[_fontIndex].base) / nn) * _fontToSprite[_fontIndex].height;
				xx = ((_letterToPrint - _fontToSprite[_fontIndex].base) % nn) * _fontToSprite[_fontIndex].width;
				_vm->_video->drawSprite(*_spritesArray[_fontToSprite[_fontIndex].sprite], *tempSrf,
										 xx, yy, xx + _fontToSprite[_fontIndex].width - 1,
										 yy + _fontToSprite[_fontIndex].height - 1, 0, 0, _transparency);
			}
			break;

		default:
			error("winDraw - Unexpected fct value %d", fct);
			break;
		}

		int i = 0;
		if (_vm->_global->_curWinId != 0)
			i = _fascinWin[_vm->_global->_curWinId].id + 1;

		for (; i < 10; i++) {
			if (table[i]) {
				int k = table[i];
				_vm->_video->drawSprite(*tempSrf, *_fascinWin[k].savedSurface,
										0, 0, width - left, height - top,
										left - _fascinWin[k].left + (_fascinWin[k].left & 7),
										top  - _fascinWin[k].top, 0);
				// Shift skipped as always set to zero (?)
				_vm->_video->drawSprite(*_frontSurface, *tempSrf,
										MAX(left  , _fascinWin[k].left),
										MAX(top   , _fascinWin[k].top),
										MIN(width , (int16) (_fascinWin[k].left + _fascinWin[k].width  - 1)),
										MIN(height, (int16) (_fascinWin[k].top  + _fascinWin[k].height - 1)),
										MAX(left  , _fascinWin[k].left) - left,
										MAX(top   , _fascinWin[k].top)  - top, 0);
				if (_cursorIndex != -1)
					_vm->_video->drawSprite(*_cursorSpritesBack, *tempSrf,
											0, 0, _cursorWidth - 1, _cursorHeight - 1,
											_cursorX - left, _cursorY - top, 0);
				for (int j = 9; j > i; j--) {
					if (table[j] && overlapWin(k, table[j])) {
						int l = table[j];
						_vm->_video->drawSprite(*_fascinWin[l].savedSurface, *tempSrf,
												MAX(_fascinWin[l].left, _fascinWin[k].left)
												  - _fascinWin[l].left + (_fascinWin[l].left & 7),
												MAX(_fascinWin[l].top , _fascinWin[k].top ) - _fascinWin[l].top,
												MIN(_fascinWin[l].left + _fascinWin[l].width  - 1, _fascinWin[k].left + _fascinWin[k].width - 1)
												  - _fascinWin[l].left + (_fascinWin[l].left & 7),
												MIN(_fascinWin[l].top  + _fascinWin[l].height - 1, _fascinWin[k].top  + _fascinWin[k].height - 1)
												  - _fascinWin[l].top,
												MAX(_fascinWin[l].left, _fascinWin[k].left) - left,
												MAX(_fascinWin[l].top , _fascinWin[k].top ) - top, 0);
					}
				}
			}
		}
		_vm->_video->drawSprite(*tempSrf, *_backSurface, 0, 0, width - left, height - top, left, top, 0);
		tempSrf.reset();
	} else {
		invalidateRect(left, top, width, height);
		switch (fct) {
		case DRAW_BLITSURF:    // 0 - move
			_vm->_video->drawSprite(*_spritesArray[_sourceSurface], *_backSurface,
									 _spriteLeft, _spriteTop,
									 _spriteLeft + _spriteRight  - 1,
									 _spriteTop  + _spriteBottom - 1,
									 _destSpriteX, _destSpriteY, _transparency);
			break;
		case DRAW_PUTPIXEL:    // 1 - put a pixel
			_vm->_video->putPixel(_destSpriteX, _destSpriteY, _frontColor, *_backSurface);
			break;

		case DRAW_FILLRECT:    // 2 - fill rectangle
			_vm->_video->fillRect(*_backSurface, _destSpriteX, _destSpriteY, _destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1, _backColor);
			break;

		case DRAW_DRAWLINE:    // 3 - draw line
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom, _frontColor);
			break;

		case DRAW_INVALIDATE:  // 4 - Draw a circle
			_vm->_video->drawCircle(*_backSurface, _spriteRight, _spriteRight, _spriteRight, _frontColor);
			break;

		case DRAW_LOADSPRITE:  // 5 - Uncompress and load a sprite
			decompWin(_destSpriteX, _destSpriteY, _backSurface);
			break;

		case DRAW_PRINTTEXT:   // 6 - Display string
			len = strlen(_textToPrint);
			for (int j = 0; j < len; j++)
				_vm->_video->drawLetter(_textToPrint[j], _destSpriteX + j * _fonts[_fontIndex]->getCharWidth(),
										_destSpriteY, *_fonts[_fontIndex], _transparency, _frontColor, _backColor, *_backSurface);
			_destSpriteX += len * _fonts[_fontIndex]->getCharWidth();
			break;

		case DRAW_DRAWBAR:     // 7 - draw border
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _spriteBottom, _spriteRight, _spriteBottom, _frontColor);
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _destSpriteY,  _destSpriteX, _spriteBottom, _frontColor);
			_vm->_video->drawLine(*_backSurface, _spriteRight, _destSpriteY,  _spriteRight, _spriteBottom, _frontColor);
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _destSpriteY,  _spriteRight, _destSpriteY,  _frontColor);
			break;

		case DRAW_CLEARRECT:   // 8 - clear rectangle
		if (_backColor < 16)
				_vm->_video->fillRect(*_backSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom, _backColor);
			break;

		case DRAW_FILLRECTABS: // 9 - fill rectangle, with other coordinates
			_vm->_video->fillRect(*_backSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom, _backColor);
			break;

		case DRAW_DRAWLETTER:  // 10 - Display a character
			if (_fontToSprite[_fontIndex].sprite == -1) {
				if (_letterToPrint)
					_vm->_video->drawLetter(_letterToPrint, _destSpriteX, _destSpriteY, *_fonts[_fontIndex], _transparency,
											_frontColor, _backColor, *_spritesArray[_destSurface]);
			} else {
				int xx, yy, nn;
				nn = _spritesArray[_fontToSprite[_fontIndex].sprite]->getWidth() / _fontToSprite[_fontIndex].width;
				yy = ((_letterToPrint - _fontToSprite[_fontIndex].base) / nn) * _fontToSprite[_fontIndex].height;
				xx = ((_letterToPrint - _fontToSprite[_fontIndex].base) % nn) * _fontToSprite[_fontIndex].width;
				_vm->_video->drawSprite(*_spritesArray[_fontToSprite[_fontIndex].sprite], *_spritesArray[_destSurface],
										xx, yy,
										xx + _fontToSprite[_fontIndex].width  - 1,
										yy + _fontToSprite[_fontIndex].height - 1,
										_destSpriteX, _destSpriteY, _transparency);
			}
			break;

		default:
			error("winDraw - Unexpected fct value");
			break;
		}
	}

	if (_renderFlags & 16) {
		if (_sourceSurface == kBackSurface) {
			_spriteLeft -= _backDeltaX;
			_spriteTop -= _backDeltaY;
		}
		if (_destSurface == kBackSurface) {
			_destSpriteX -= _backDeltaX;
			_destSpriteY -= _backDeltaY;
		}
	}

	if (_vm->_global->_curWinId) {
		_destSpriteX -= _fascinWin[_vm->_global->_curWinId].left;
		_destSpriteY -= _fascinWin[_vm->_global->_curWinId].top;
	}
}

void Draw_Fascination::decompWin(int16 x, int16 y, SurfaceDescPtr destPtr) {
	Resource *resource;
	resource = _vm->_game->_resources->getResource((uint16) _spriteLeft,
	                                               &_spriteRight, &_spriteBottom);

	if (!resource)
		return;

	_vm->_video->drawPackedSprite(resource->getData(),
			_spriteRight, _spriteBottom, x, y, _transparency, *destPtr);

	delete resource;
	return;
}

int16 Draw_Fascination::openWin(int16 id) {
	if (_fascinWin[id].id != -1)
		return 0;

	_fascinWin[id].id = _winCount++;
	_fascinWin[id].left   = VAR((_winVarArrayLeft   / 4) + id);
	_fascinWin[id].top    = VAR((_winVarArrayTop    / 4) + id);
	_fascinWin[id].width  = VAR((_winVarArrayWidth  / 4) + id);
	_fascinWin[id].height = VAR((_winVarArrayHeight / 4) + id);

	_fascinWin[id].savedSurface = _vm->_video->initSurfDesc(_vm->_global->_videoMode, _winMaxWidth + 7, _winMaxHeight, 0);

	saveWin(id);
	WRITE_VAR((_winVarArrayStatus / 4) + id, VAR((_winVarArrayStatus / 4) + id) & 0xFFFFFFFE);

	return 1;
}

int16 Draw_Fascination::getWinFromCoord(int16 &dx, int16 &dy) {
	int16 bestMatch = -1;

	if ((_renderFlags & 128) == 0)
		return -1;

	for (int i = 0; i < 10; i++) {
		if (_fascinWin[i].id != -1) {
			if ((_vm->_global->_inter_mouseX >= _fascinWin[i].left) &&
			    (_vm->_global->_inter_mouseX <  _fascinWin[i].left + _fascinWin[i].width) &&
			    (_vm->_global->_inter_mouseY >= _fascinWin[i].top) &&
			    (_vm->_global->_inter_mouseY <  _fascinWin[i].top  + _fascinWin[i].height)) {

				if (_fascinWin[i].id == _winCount - 1) {
					dx = _fascinWin[i].left;
					dy = _fascinWin[i].top;
					return(i);
				} else {
					if (_fascinWin[i].id > bestMatch)
						bestMatch = _fascinWin[i].id;
				}
			}
		}
	}

	if (bestMatch != -1)
		return(0);
	else
		return(-1);
}

void Draw_Fascination::closeWin(int16 id) {
	if (_fascinWin[id].id == -1)
		return;

	WRITE_VAR((_winVarArrayStatus / 4) + id, VAR((_winVarArrayStatus / 4) + id) | 1);
	restoreWin(id);
	_fascinWin[id].id = -1;
	_fascinWin[id].savedSurface.reset();
	_winCount--;
}

int16 Draw_Fascination::handleCurWin() {
	int8 matchNum = 0;
	int16 bestMatch = -1;

	if ((_vm->_game->_mouseButtons != 1) || ((_renderFlags & 128) == 0))
		return 0;

	for (int i = 0; i < 10; i++) {
		if (_fascinWin[i].id != -1) {
			if ((_vm->_global->_inter_mouseX >= _fascinWin[i].left) &&
			    (_vm->_global->_inter_mouseX <  _fascinWin[i].left + _fascinWin[i].width) &&
			    (_vm->_global->_inter_mouseY >= _fascinWin[i].top) &&
			    (_vm->_global->_inter_mouseY <  _fascinWin[i].top  + _fascinWin[i].height)) {

				if (_fascinWin[i].id == _winCount - 1) {
					if ((_vm->_global->_inter_mouseX < _fascinWin[i].left + 12) &&
					    (_vm->_global->_inter_mouseY < _fascinWin[i].top  + 12) &&
					    (VAR(_winVarArrayStatus / 4 + i) & 2)) {

						blitCursor();
						activeWin(i);
						closeWin(i);
						_vm->_util->waitMouseRelease(1);
						return i;
					}

					if ((_vm->_global->_inter_mouseX >= _fascinWin[i].left + _fascinWin[i].width - 12) &&
					    (_vm->_global->_inter_mouseY <  _fascinWin[i].top  + 12) &&
					    (VAR(_winVarArrayStatus / 4 + i) & 4) &&
					    (_vm->_global->_mousePresent) &&
					    (_vm->_global->_videoMode != 0x07)) {

						blitCursor();
						handleWinBorder(i);
						moveWin(i);
						_vm->_global->_inter_mouseX = _fascinWin[i].left + _fascinWin[i].width - 11;
						_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
						return -i;
					}
					return 0;
				} else {
					if (_fascinWin[i].id > bestMatch) {
						bestMatch = _fascinWin[i].id;
						matchNum = i;
					}
				}
			}
		}
	}

	if (bestMatch != -1) {
		blitCursor();
		activeWin(matchNum);
	}

	return 0;
}

void Draw_Fascination::moveWin(int16 id) {
	int oldLeft = _fascinWin[id].left;
	int oldTop  = _fascinWin[id].top;

	restoreWin(id);

	_fascinWin[id].left = _vm->_global->_inter_mouseX;
	_fascinWin[id].top  = _vm->_global->_inter_mouseY;

	WRITE_VAR((_winVarArrayLeft / 4) + id, _fascinWin[id].left);
	WRITE_VAR((_winVarArrayTop  / 4) + id, _fascinWin[id].top);

	saveWin(id);

	// Shift skipped as always set to zero (?)
	_vm->_video->drawSprite(*_frontSurface, *_backSurface,
							oldLeft, oldTop,
							oldLeft + _fascinWin[id].width  - 1,
							oldTop  + _fascinWin[id].height - 1,
							_fascinWin[id].left, _fascinWin[id].top, 0);
	invalidateRect(_fascinWin[id].left, _fascinWin[id].top,
				   _fascinWin[id].left + _fascinWin[id].width  - 1,
				   _fascinWin[id].top  + _fascinWin[id].height - 1);
}

bool Draw_Fascination::overlapWin(int16 idWin1, int16 idWin2) {
	if ((_fascinWin[idWin1].left + _fascinWin[idWin1].width  <= _fascinWin[idWin2].left) ||
		(_fascinWin[idWin2].left + _fascinWin[idWin2].width  <= _fascinWin[idWin1].left) ||
		(_fascinWin[idWin1].top  + _fascinWin[idWin1].height <= _fascinWin[idWin2].top ) ||
		(_fascinWin[idWin2].top  + _fascinWin[idWin2].height <= _fascinWin[idWin1].top ))
		return false;

	return true;
}

void Draw_Fascination::activeWin(int16 id) {
	bool found = false;
	int16 t[10], t2[10];
	int nextId = -1;
	int oldId  = -1;
	SurfaceDescPtr tempSrf;
	SurfaceDescPtr oldSrf[10];

	if (_fascinWin[id].id == -1)
		return;

	blitInvalidated();

	for (int i = 0; i < 10; i++) {
		t[i]  = -1;
		t2[i] = -1;
		oldSrf[i].reset();
	}

	for (int i = 0; i < 10; i++) {
		if ((i != id) && (_fascinWin[i].id > _fascinWin[id].id) && (overlapWin(i, id))) {
			t[_fascinWin[i].id] = i;
			found = true;
		}
	}

	if (found) {
		for (int i = 9; i >= 0; i--) {
			if (t[i] != -1) {
				if (nextId != -1)
					_vm->_video->drawSprite(*_backSurface, *_fascinWin[nextId].savedSurface,
											_fascinWin[t[i]].left, _fascinWin[t[i]].top,
											_fascinWin[t[i]].left + _fascinWin[t[i]].width  - 1,
											_fascinWin[t[i]].top  + _fascinWin[t[i]].height - 1,
											_fascinWin[t[i]].left & 7, 0, 0);
				t2[i] = nextId;
				restoreWin(t[i]);
				nextId = t[i];
			}
		}

		oldId = nextId;
		_vm->_video->drawSprite(*_backSurface, *_fascinWin[nextId].savedSurface,
								 _fascinWin[id].left, _fascinWin[id].top,
								 _fascinWin[id].left + _fascinWin[id].width  - 1,
								 _fascinWin[id].top  + _fascinWin[id].height - 1,
								 _fascinWin[id].left & 7, 0, 0);
		restoreWin(id);
		nextId = id;

		for (int i = 0; i < 10; i++) {
			if (t[i] != -1) {
				_vm->_video->drawSprite(*_backSurface, *_fascinWin[nextId].savedSurface,
										_fascinWin[t[i]].left, _fascinWin[t[i]].top,
										_fascinWin[t[i]].left + _fascinWin[t[i]].width  - 1,
										_fascinWin[t[i]].top  + _fascinWin[t[i]].height - 1,
										_fascinWin[t[i]].left & 7, 0, 0);
				oldSrf[t[i]] = _fascinWin[nextId].savedSurface;
				if (t2[i] != -1)
					_vm->_video->drawSprite(*_fascinWin[t2[i]].savedSurface, *_backSurface,
											 _fascinWin[t[i]].left & 7, 0,
											(_fascinWin[t[i]].left & 7) + _fascinWin[t[i]].width - 1,
											 _fascinWin[t[i]].height - 1, _fascinWin[t[i]].left,
											 _fascinWin[t[i]].top, 0);
				else {
					// Shift skipped as always set to zero (?)
					_vm->_video->drawSprite(*_frontSurface, *_backSurface,
											 _fascinWin[t[i]].left, _fascinWin[t[i]].top,
											 _fascinWin[t[i]].left + _fascinWin[t[i]].width  - 1,
											 _fascinWin[t[i]].top  + _fascinWin[t[i]].height - 1,
											 _fascinWin[t[i]].left, _fascinWin[t[i]].top, 0);
				}
				invalidateRect(_fascinWin[t[i]].left, _fascinWin[t[i]].top,
							_fascinWin[t[i]].left + _fascinWin[t[i]].width  - 1,
							_fascinWin[t[i]].top  + _fascinWin[t[i]].height - 1);
				nextId = t2[i];
			}
		}

		tempSrf = _vm->_video->initSurfDesc(_vm->_global->_videoMode, _winMaxWidth + 7, _winMaxHeight, 0);
		_vm->_video->drawSprite(*_backSurface, *tempSrf,
								 _fascinWin[id].left, _fascinWin[id].top,
								 _fascinWin[id].left + _fascinWin[id].width  - 1,
								 _fascinWin[id].top  + _fascinWin[id].height - 1,
								 _fascinWin[id].left & 7, 0, 0);
		_vm->_video->drawSprite(*_fascinWin[oldId].savedSurface, *_backSurface,
								 _fascinWin[id].left & 7, 0,
								(_fascinWin[id].left & 7) + _fascinWin[id].width - 1,
								 _fascinWin[id].height - 1,
								 _fascinWin[id].left, _fascinWin[id].top, 0);

		_fascinWin[oldId].savedSurface.reset();
		_fascinWin[oldId].savedSurface = tempSrf;
		oldSrf[id] = _fascinWin[oldId].savedSurface;

		invalidateRect(_fascinWin[id].left, _fascinWin[id].top,
					_fascinWin[id].left + _fascinWin[id].width  - 1,
					_fascinWin[id].top  + _fascinWin[id].height - 1);
		nextId = id;

		for (int j = 0; j < 10; j++) {
			if (oldSrf[j] != 0)
				_fascinWin[j].savedSurface = oldSrf[j];
		}
	}

	for (int i = 0; i < 10; i++) {
		if ((i != id) && (_fascinWin[i].id > _fascinWin[id].id))
			_fascinWin[i].id--;
	}

	_fascinWin[id].id = _winCount - 1;
}

void Draw_Fascination::closeAllWin() {
	for (int i = 0; i < 10; i++) {
		activeWin(i);
		closeWin(i);
	}
}

void Draw_Fascination::saveWin(int16 id) {
	_vm->_video->drawSprite(*_backSurface, *_fascinWin[id].savedSurface,
							_fascinWin[id].left,  _fascinWin[id].top,
							_fascinWin[id].left + _fascinWin[id].width  - 1,
							_fascinWin[id].top  + _fascinWin[id].height - 1,
							_fascinWin[id].left & 7, 0, 0);
}

void Draw_Fascination::restoreWin(int16 id) {
	_vm->_video->drawSprite(*_fascinWin[id].savedSurface, *_backSurface,
							 _fascinWin[id].left & 7, 0,
							(_fascinWin[id].left & 7) + _fascinWin[id].width - 1, _fascinWin[id].height - 1,
							 _fascinWin[id].left, _fascinWin[id].top, 0);
	invalidateRect(_fascinWin[id].left, _fascinWin[id].top,
				_fascinWin[id].left + _fascinWin[id].width  - 1,
				_fascinWin[id].top  + _fascinWin[id].height - 1);
}

void Draw_Fascination::drawWinTrace(int16 left, int16 top, int16 width, int16 height) {
	int16 right, bottom;

	right  = left + width  - 1;
	bottom = top  + height - 1;

	for (int32 x = left; x < right; x++) {
		_frontSurface->getVidMem()[_frontSurface->getWidth() * top + x] = (128 + _frontSurface->getVidMem()[_frontSurface->getWidth() * top + x]) & 0xff;
		_frontSurface->getVidMem()[_frontSurface->getWidth() * bottom + x] = (128 + _frontSurface->getVidMem()[_frontSurface->getWidth() * bottom + x]) & 0xff;
	}

	for (int32 y = top; y < bottom; y++) {
		_frontSurface->getVidMem()[_frontSurface->getWidth() * y + left] = (128 + _frontSurface->getVidMem()[_frontSurface->getWidth() * y + left]) & 0xff;
		_frontSurface->getVidMem()[_frontSurface->getWidth() * y + right] = (128 + _frontSurface->getVidMem()[_frontSurface->getWidth() * y + right]) & 0xff;
	}

	_vm->_video->dirtyRectsAll();
	_vm->_video->retrace(true);
}

void Draw_Fascination::handleWinBorder(int16 id) {
	int16 minX = 0;
	int16 maxX = 320;
	int16 minY = 0;
	int16 maxY = 200;

	if (VAR((_winVarArrayStatus / 4) + id) & 8)
		minX = (int16)(VAR((_winVarArrayLimitsX / 4) + id) >> 16L);
	if (VAR((_winVarArrayStatus / 4) + id) & 16)
		maxX = (int16)(VAR((_winVarArrayLimitsX / 4) + id) & 0xFFFFL);
	if (VAR((_winVarArrayStatus / 4) + id) & 32)
		minY = (int16)(VAR((_winVarArrayLimitsY / 4) + id) >> 16L);
	if (VAR((_winVarArrayStatus / 4) + id) & 64)
		maxY = (int16)(VAR((_winVarArrayLimitsY / 4) + id) & 0xFFFFL);

	_vm->_global->_inter_mouseX = _fascinWin[id].left;
	_vm->_global->_inter_mouseY = _fascinWin[id].top;

	if (_vm->_global->_mousePresent)
		_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);

	drawWinTrace(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY, _fascinWin[id].width, _fascinWin[id].height);
	_cursorX = _vm->_global->_inter_mouseX;
	_cursorY = _vm->_global->_inter_mouseY;

	do {
		_vm->_game->checkKeys(&_vm->_global->_inter_mouseX, &_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 1);

		if (_vm->_global->_inter_mouseX != _cursorX || _vm->_global->_inter_mouseY != _cursorY) {
			if (_vm->_global->_inter_mouseX < minX) {
				_vm->_global->_inter_mouseX = minX;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}

			if (_vm->_global->_inter_mouseY < minY) {
				_vm->_global->_inter_mouseY = minY;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}

			if (_vm->_global->_inter_mouseX + _fascinWin[id].width > maxX) {
				_vm->_global->_inter_mouseX = maxX - _fascinWin[id].width;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}

			if (_vm->_global->_inter_mouseY + _fascinWin[id].height > maxY) {
				_vm->_global->_inter_mouseY = maxY - _fascinWin[id].height;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}

			drawWinTrace(_cursorX, _cursorY, _fascinWin[id].width, _fascinWin[id].height);
			drawWinTrace(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY, _fascinWin[id].width, _fascinWin[id].height);
			_cursorX = _vm->_global->_inter_mouseX;
			_cursorY = _vm->_global->_inter_mouseY;
		}
	} while (_vm->_game->_mouseButtons);
	drawWinTrace(_cursorX, _cursorY, _fascinWin[id].width, _fascinWin[id].height);
	_cursorX = _vm->_global->_inter_mouseX;
	_cursorY = _vm->_global->_inter_mouseY;
}

} // End of namespace Gob
